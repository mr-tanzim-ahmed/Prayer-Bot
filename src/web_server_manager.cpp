#include "web_server_manager.h"
#include "config.h"
#include "dashboard_html.h"
#include "storage_manager.h"
#include "prayer_manager.h"
#include "weather_manager.h"
#include "pomodoro_manager.h"
#include "calendar_manager.h"
#include "screens/dhikr_screen.h"
#include "pomodoro_stats.h"
#include "wifi_manager.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>
#include <math.h>

extern volatile bool g_locationRefreshRequested;
extern volatile bool g_weatherRefreshRequested;
extern volatile bool g_ntpSyncRequested;
extern PomodoroState g_pomodoro;
extern volatile int16_t g_webVolumeRequest;

// ============================================================
//  WEB SERVER MANAGER IMPLEMENTATION
//  Serves the local device dashboard and its JSON API.
// ============================================================

namespace {
    AsyncWebServer* server = nullptr;
    bool serverRunning = false;
    Settings* settingsRef = nullptr;
    constexpr size_t MAX_REQUEST_BODY_BYTES = 2048;

    bool validInteger(JsonVariantConst value, int minimum, int maximum) {
        if (value.isUnbound()) return true;
        if (!value.is<int>()) return false;
        const int number = value.as<int>();
        return number >= minimum && number <= maximum;
    }

    bool validCalculationMethod(JsonVariantConst value) {
        if (value.isUnbound()) return true;
        if (!value.is<int>()) return false;
        const int method = value.as<int>();
        return (method >= 0 && method <= 5) ||
               (method >= 7 && method <= 23) || method == 99;
    }

    bool isNumber(JsonVariantConst value) {
        return value.is<int>() || value.is<float>();
    }

    bool validNumber(JsonVariantConst value, float minimum, float maximum) {
        if (value.isUnbound()) return true;
        if (!isNumber(value)) return false;
        const float number = value.as<float>();
        return isfinite(number) && number >= minimum && number <= maximum;
    }

    bool validString(JsonVariantConst value, size_t maxLength, bool allowEmpty = false) {
        if (value.isUnbound()) return true;
        if (!value.is<const char*>()) return false;
        const char* text = value.as<const char*>();
        const size_t length = strlen(text);
        return length <= maxLength && (allowEmpty || length > 0);
    }

    bool hasOnlyKeys(JsonObjectConst object, const char* const* allowed, size_t allowedCount) {
        for (JsonPairConst member : object) {
            const char* key = member.key().c_str();
            bool found = false;
            for (size_t i = 0; i < allowedCount; ++i) {
                if (strcmp(key, allowed[i]) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
        return true;
    }

    bool validSettings(const JsonDocument& doc, String& error) {
        if (!validString(doc["city"], sizeof(settingsRef->cityName) - 1)) {
            error = "City is required and must be 1-63 characters.";
        } else if (!validString(doc["timezone"], sizeof(settingsRef->timezone) - 1)) {
            error = "Timezone is required and must be 1-39 characters.";
        } else if (!validNumber(doc["lat"], -90.0f, 90.0f)) {
            error = "Latitude must be between -90 and 90.";
        } else if (!validNumber(doc["lon"], -180.0f, 180.0f)) {
            error = "Longitude must be between -180 and 180.";
        } else if (!validNumber(doc["utcOff"], -12.0f, 14.0f)) {
            error = "UTC offset must be between -12 and 14.";
        } else if (!validCalculationMethod(doc["calcMethod"]) ||
                   !validInteger(doc["asrSchool"], 0, 1) ||
                   !validInteger(doc["hijriOffset"], -2, 2) ||
                   !validInteger(doc["volume"], 0, 100) ||
                   !validInteger(doc["focusMin"], 1, 120) ||
                   !validInteger(doc["shortBrk"], 2, 5) ||
                   !validInteger(doc["longBrk"], 1, 60) ||
                   !validInteger(doc["pomCycles"], 1, 10) ||
                   !validInteger(doc["sunriseOff"], 0, 60) ||
                   !validInteger(doc["zawalOff"], 0, 60) ||
                   !validInteger(doc["sunsetOff"], 0, 60)) {
            error = "One or more settings are outside their allowed range.";
        } else if (!validString(doc["owmKey"], sizeof(settingsRef->owmApiKey) - 1, true)) {
            error = "OpenWeather API key must be at most 47 characters.";
        } else if (doc["azanOn"].isUnbound() == false &&
                   !doc["azanOn"].is<bool>()) {
            error = "Azan enabled must be true or false.";
        }
        return error.isEmpty();
    }

    String formatTime(const PrayerTime& time) {
        char text[6];
        snprintf(text, sizeof(text), "%02u:%02u", time.hour, time.minute);
        return String(text);
    }

    String formatDuration(int32_t seconds) {
        if (seconds < 0) seconds = 0;
        char text[16];
        snprintf(text, sizeof(text), "%02ld:%02ld:%02ld",
                 (long)(seconds / 3600),
                 (long)((seconds % 3600) / 60),
                 (long)(seconds % 60));
        return String(text);
    }

    const char* pomodoroPhaseName(PomodoroPhase phase) {
        switch (phase) {
            case POMODORO_FOCUS: return "Focus";
            case POMODORO_SHORT_BREAK: return "Short break";
            case POMODORO_LONG_BREAK: return "Long break";
            case POMODORO_PAUSED: return "Paused";
            case POMODORO_WAITING_FOR_BREAK: return "Waiting for touch";
            default: return "Idle";
        }
    }

    void handleSettings(AsyncWebServerRequest* request, const String& body) {
        JsonDocument doc;
        if (deserializeJson(doc, body)) {
            request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
            return;
        }
        if (!doc.is<JsonObject>()) {
            request->send(400, "application/json", "{\"error\":\"Settings body must be a JSON object.\"}");
            return;
        }
        static const char* const settingKeys[] = {
            "city", "timezone", "lat", "lon", "utcOff", "calcMethod",
            "asrSchool", "hijriOffset", "azanOn", "volume", "focusMin",
            "shortBrk", "longBrk", "pomCycles", "sunriseOff", "zawalOff",
            "sunsetOff", "owmKey"
        };
        if (!hasOnlyKeys(doc.as<JsonObjectConst>(), settingKeys,
                         sizeof(settingKeys) / sizeof(settingKeys[0]))) {
            request->send(400, "application/json", "{\"error\":\"Settings contain an unknown field.\"}");
            return;
        }

        String validationError;
        if (!validSettings(doc, validationError)) {
            JsonDocument response;
            response["error"] = validationError;
            String json;
            serializeJson(response, json);
            request->send(400, "application/json", json);
            return;
        }

        const bool locationChanged =
            (isNumber(doc["lat"]) && doc["lat"].as<float>() != settingsRef->latitude) ||
            (isNumber(doc["lon"]) && doc["lon"].as<float>() != settingsRef->longitude) ||
            (doc["calcMethod"].is<int>() && doc["calcMethod"].as<int>() != settingsRef->calcMethod) ||
            (doc["asrSchool"].is<int>() && doc["asrSchool"].as<int>() != settingsRef->asrSchool) ||
            (doc["city"].is<const char*>() &&
             String(doc["city"].as<const char*>()) != String(settingsRef->cityName));
        const bool timezoneChanged =
            (isNumber(doc["utcOff"]) && doc["utcOff"].as<float>() != settingsRef->utcOffset) ||
            (doc["timezone"].is<const char*>() &&
             String(doc["timezone"].as<const char*>()) != String(settingsRef->timezone));
        const bool weatherConfigChanged =
            (doc["owmKey"].is<const char*>() &&
             String(doc["owmKey"].as<const char*>()) != String(settingsRef->owmApiKey));

        Settings previousSettings = *settingsRef;
        if (doc["azanOn"].is<bool>())
            settingsRef->azanEnabled = doc["azanOn"].as<bool>();
        if (doc["volume"].is<int>())
            settingsRef->volume = doc["volume"].as<uint8_t>();
        if (doc["owmKey"].is<const char*>()) {
            const char* apiKey = doc["owmKey"].as<const char*>();
            if (apiKey[0] != '\0')
                strlcpy(settingsRef->owmApiKey, apiKey, sizeof(settingsRef->owmApiKey));
        }
        if (doc["city"].is<const char*>())
            strlcpy(settingsRef->cityName, doc["city"].as<const char*>(), sizeof(settingsRef->cityName));
        if (doc["timezone"].is<const char*>())
            strlcpy(settingsRef->timezone, doc["timezone"].as<const char*>(), sizeof(settingsRef->timezone));
        if (isNumber(doc["lat"])) settingsRef->latitude = doc["lat"].as<float>();
        if (isNumber(doc["lon"])) settingsRef->longitude = doc["lon"].as<float>();
        if (isNumber(doc["utcOff"])) settingsRef->utcOffset = doc["utcOff"].as<float>();
        if (doc["calcMethod"].is<int>()) settingsRef->calcMethod = doc["calcMethod"].as<uint8_t>();
        if (doc["asrSchool"].is<int>()) settingsRef->asrSchool = doc["asrSchool"].as<uint8_t>();
        if (doc["hijriOffset"].is<int>()) settingsRef->hijriOffset = doc["hijriOffset"].as<int8_t>();
        if (doc["focusMin"].is<int>()) settingsRef->focusMinutes = doc["focusMin"].as<uint16_t>();
        if (doc["shortBrk"].is<int>()) settingsRef->shortBreakMinutes = doc["shortBrk"].as<uint16_t>();
        if (doc["longBrk"].is<int>()) settingsRef->longBreakMinutes = doc["longBrk"].as<uint16_t>();
        if (doc["pomCycles"].is<int>()) settingsRef->pomodoroCycles = doc["pomCycles"].as<uint8_t>();
        if (doc["sunriseOff"].is<int>()) settingsRef->sunriseOffset = doc["sunriseOff"].as<uint8_t>();
        if (doc["zawalOff"].is<int>()) settingsRef->zawalOffset = doc["zawalOff"].as<uint8_t>();
        if (doc["sunsetOff"].is<int>()) settingsRef->sunsetOffset = doc["sunsetOff"].as<uint8_t>();

        if (!StorageManager::saveSettings(*settingsRef)) {
            *settingsRef = previousSettings;
            PomodoroManager::updateSettings(g_pomodoro, *settingsRef);
            request->send(500, "application/json", "{\"error\":\"Could not save settings to device storage.\"}");
            return;
        }

        g_webVolumeRequest = settingsRef->volume;
        PomodoroManager::updateSettings(g_pomodoro, *settingsRef);
        CalendarManager::setHijriOffset(settingsRef->hijriOffset);
        if (timezoneChanged) g_ntpSyncRequested = true;
        if (locationChanged) {
            g_locationRefreshRequested = true;
            g_weatherRefreshRequested = true;
        }
        if (weatherConfigChanged) g_weatherRefreshRequested = true;

        request->send(200, "application/json", "{\"status\":\"ok\"}");
        Serial.println("[WEB] Settings saved.");
    }

    String* appendRequestBody(AsyncWebServerRequest* request,
                              uint8_t* data, size_t len,
                              size_t index, size_t total) {
        if (index == 0) {
            delete static_cast<String*>(request->_tempObject);
            if (total > MAX_REQUEST_BODY_BYTES) {
                request->send(413, "application/json", "{\"error\":\"Request body is too large.\"}");
                request->_tempObject = nullptr;
                return nullptr;
            }
            request->_tempObject = new String();
        }
        String* body = static_cast<String*>(request->_tempObject);
        if (!body) return nullptr;
        body->reserve(total);
        for (size_t i = 0; i < len; ++i) body->concat((char)data[i]);
        if (index + len != total) return nullptr;
        request->_tempObject = nullptr;
        return body;
    }
}

extern WeatherData g_weather;
extern DailyPrayers g_todayPrayers;
extern NextPrayerInfo g_nextPrayer;
extern ProhibitedTimes g_prohibited;
extern HijriDate g_hijriDate;
extern IslamicEvent g_todayEvent;
extern PomodoroState g_pomodoro;
extern SystemScreen g_currentScreen;
extern volatile AzanState g_azanState;
extern bool g_weatherReady;
extern bool g_prayerReady;
extern volatile int8_t g_webScreenRequest;
extern volatile uint8_t g_webPomodoroRequest;
extern volatile uint8_t g_webDhikrRequest;
extern volatile bool g_azanStopRequested;

namespace WebServerManager {

void init(Settings& settings) {
    if (server) {
        server->end();
        delete server;
    }

    settingsRef = &settings;
    server = new AsyncWebServer(WEB_SERVER_PORT);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", DASHBOARD_HTML);
    });

    server->on("/api/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        doc["azanOn"] = settingsRef->azanEnabled;
        doc["volume"] = settingsRef->volume;
        doc["city"] = settingsRef->cityName;
        doc["lat"] = settingsRef->latitude;
        doc["lon"] = settingsRef->longitude;
        doc["timezone"] = settingsRef->timezone;
        doc["utcOff"] = settingsRef->utcOffset;
        doc["calcMethod"] = settingsRef->calcMethod;
        doc["asrSchool"] = settingsRef->asrSchool;
        doc["hijriOffset"] = settingsRef->hijriOffset;
        doc["focusMin"] = settingsRef->focusMinutes;
        doc["shortBrk"] = settingsRef->shortBreakMinutes;
        doc["longBrk"] = settingsRef->longBreakMinutes;
        doc["pomCycles"] = settingsRef->pomodoroCycles;
        doc["sunriseOff"] = settingsRef->sunriseOffset;
        doc["zawalOff"] = settingsRef->zawalOffset;
        doc["sunsetOff"] = settingsRef->sunsetOffset;
        doc["owmKeyConfigured"] = settingsRef->owmApiKey[0] != '\0';
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    server->on("/api/settings", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String* body = appendRequestBody(request, data, len, index, total);
            if (!body) return;
            handleSettings(request, *body);
            delete body;
        });

    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        JsonObject network = doc["network"].to<JsonObject>();
        network["connected"] = WiFi.status() == WL_CONNECTED;
        network["mode"] = (WiFi.getMode() & WIFI_AP)
            ? (WiFi.status() == WL_CONNECTED ? "access-point+station" : "access-point")
            : "station";
        network["setupRequired"] =
            (WiFi.getMode() & WIFI_AP) && WiFi.status() != WL_CONNECTED;
        network["connecting"] = WifiManager::isConnecting();
        network["ip"] = (WiFi.getMode() & WIFI_AP)
            ? WiFi.softAPIP().toString()
            : WiFi.localIP().toString();
        network["homeIp"] = WiFi.status() == WL_CONNECTED
            ? WiFi.localIP().toString()
            : "";

        JsonObject prayers = doc["prayers"].to<JsonObject>();
        prayers["ready"] = g_prayerReady;
        prayers["fajr"] = formatTime(g_todayPrayers.fajr);
        prayers["sunrise"] = formatTime(g_todayPrayers.sunrise);
        prayers["dhuhr"] = formatTime(g_todayPrayers.dhuhr);
        prayers["asr"] = formatTime(g_todayPrayers.asr);
        prayers["maghrib"] = formatTime(g_todayPrayers.maghrib);
        prayers["isha"] = formatTime(g_todayPrayers.isha);

        JsonObject next = doc["nextPrayer"].to<JsonObject>();
        next["name"] = g_prayerReady ? PrayerManager::getPrayerNameStr(g_nextPrayer.name) : "";
        next["time"] = formatTime(g_nextPrayer.startTime);
        next["countdown"] = formatDuration(g_nextPrayer.countdownSeconds);
        doc["azanPlaying"] = g_azanState == AZAN_PLAYING;

        JsonObject hijri = doc["hijri"].to<JsonObject>();
        char hijriText[64];
        snprintf(hijriText, sizeof(hijriText), "%u %s %u %s",
                 g_hijriDate.day, g_hijriDate.monthName.c_str(),
                 g_hijriDate.year, g_hijriDate.designation.c_str());
        hijri["date"] = g_hijriDate.year ? hijriText : "";
        hijri["event"] = g_todayEvent.name;

        JsonObject weather = doc["weather"].to<JsonObject>();
        weather["valid"] = g_weatherReady && g_weather.valid;
        weather["city"] = g_weather.cityName;
        weather["temperatureC"] = g_weather.temperatureC;
        weather["humidity"] = g_weather.humidity;
        weather["aqi"] = g_weather.airQualityIndex;
        weather["pm25"] = g_weather.pm25;

        JsonObject pomodoro = doc["pomodoro"].to<JsonObject>();
        pomodoro["phase"] = pomodoroPhaseName(g_pomodoro.phase);
        pomodoro["remaining"] = formatDuration(g_pomodoro.remainingSeconds);
        pomodoro["nextBreakMinutes"] =
            g_pomodoro.pendingBreakPhase == POMODORO_LONG_BREAK
                ? g_pomodoro.longBreakMinutes
                : g_pomodoro.shortBreakMinutes;
        pomodoro["cycle"] = g_pomodoro.currentCycle >= g_pomodoro.totalCycles
            ? g_pomodoro.totalCycles
            : g_pomodoro.currentCycle + 1;
        pomodoro["cycles"] = g_pomodoro.totalCycles;
        doc["screen"] = (uint8_t)g_currentScreen;
        doc["dhikrCount"] = DhikrScreen::getCount();

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    server->on("/api/stats", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        if (!PomodoroStats::getWeeklyStats(doc)) {
            request->send(503, "application/json", "{\"error\":\"Weekly statistics require a valid local date and writable storage.\"}");
            return;
        }
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    server->on("/api/control", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String* body = appendRequestBody(request, data, len, index, total);
            if (!body) return;

            JsonDocument doc;
            if (deserializeJson(doc, *body)) {
                delete body;
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            delete body;
            if (!doc.is<JsonObject>()) {
                request->send(400, "application/json", "{\"error\":\"Control body must be a JSON object.\"}");
                return;
            }
            static const char* const controlKeys[] = {
                "screen", "pomodoro", "dhikr", "stopAzan"
            };
            if (!hasOnlyKeys(doc.as<JsonObjectConst>(), controlKeys,
                             sizeof(controlKeys) / sizeof(controlKeys[0]))) {
                request->send(400, "application/json", "{\"error\":\"Control contains an unknown field.\"}");
                return;
            }

            JsonVariant screenValue = doc["screen"];
            JsonVariant pomodoroValue = doc["pomodoro"];
            JsonVariant dhikrValue = doc["dhikr"];
            JsonVariant stopAzanValue = doc["stopAzan"];
            if ((!screenValue.isUnbound() && !screenValue.is<int>()) ||
                (!pomodoroValue.isUnbound() && !pomodoroValue.is<const char*>()) ||
                (!dhikrValue.isUnbound() && !dhikrValue.is<const char*>()) ||
                (!stopAzanValue.isUnbound() && !stopAzanValue.is<bool>())) {
                request->send(400, "application/json", "{\"error\":\"Control fields have invalid types.\"}");
                return;
            }

            int screen = -1;
            if (!screenValue.isUnbound()) {
                screen = screenValue.as<int>();
                if (screen < 0 || screen >= SYS_SCREEN_COUNT) {
                    request->send(400, "application/json", "{\"error\":\"Unknown screen.\"}");
                    return;
                }
            }

            uint8_t pomodoroCommand = 0;
            if (!pomodoroValue.isUnbound()) {
                const char* action = pomodoroValue.as<const char*>();
                if (strcmp(action, "toggle") == 0) pomodoroCommand = 1;
                else if (strcmp(action, "reset") == 0) pomodoroCommand = 2;
                else {
                    request->send(400, "application/json", "{\"error\":\"Unknown Pomodoro action.\"}");
                    return;
                }
            }

            bool countDhikr = false;
            if (!dhikrValue.isUnbound()) {
                if (strcmp(dhikrValue.as<const char*>(), "count") != 0) {
                    request->send(400, "application/json", "{\"error\":\"Unknown zikir action.\"}");
                    return;
                }
                countDhikr = true;
            }

            const bool stopAzan = !stopAzanValue.isUnbound() && stopAzanValue.as<bool>();
            const bool hasCommand = screen >= 0 || pomodoroCommand != 0 ||
                                    countDhikr || stopAzan;
            if (!hasCommand) {
                request->send(400, "application/json", "{\"error\":\"No supported device command was provided.\"}");
                return;
            }

            if (screen >= 0) g_webScreenRequest = (int8_t)screen;
            if (pomodoroCommand != 0) g_webPomodoroRequest = pomodoroCommand;
            if (countDhikr) g_webDhikrRequest = 1;
            if (stopAzan) g_azanStopRequested = true;
            request->send(200, "application/json", "{\"status\":\"queued\"}");
        });

    server->on("/api/wifi", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String* body = appendRequestBody(request, data, len, index, total);
            if (!body) return;

            JsonDocument doc;
            if (deserializeJson(doc, *body)) {
                delete body;
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            delete body;
            if (!doc.is<JsonObject>()) {
                request->send(400, "application/json", "{\"error\":\"Wi-Fi body must be a JSON object.\"}");
                return;
            }
            static const char* const wifiKeys[] = {"ssid", "pass"};
            if (!hasOnlyKeys(doc.as<JsonObjectConst>(), wifiKeys,
                             sizeof(wifiKeys) / sizeof(wifiKeys[0]))) {
                request->send(400, "application/json", "{\"error\":\"Wi-Fi settings contain an unknown field.\"}");
                return;
            }

            if (!validString(doc["ssid"], 32) || !validString(doc["pass"], 63, true)) {
                request->send(400, "application/json", "{\"error\":\"Enter an SSID (up to 32 characters) and password (up to 63 characters).\"}");
                return;
            }
            String ssid = doc["ssid"].as<String>();
            String password = doc["pass"].as<String>();
            if (!StorageManager::writeFile("/wifi_ssid.txt", ssid) ||
                !StorageManager::writeFile("/wifi_pass.txt", password)) {
                request->send(500, "application/json", "{\"error\":\"Could not save Wi-Fi credentials.\"}");
                return;
            }

            if (!WifiManager::connectToNetwork(ssid, password)) {
                request->send(400, "application/json", "{\"error\":\"Could not start the Wi-Fi connection.\"}");
                return;
            }
            request->send(200, "application/json", "{\"status\":\"connecting\",\"message\":\"Connecting to Wi-Fi...\"}");
        });

    server->onNotFound([](AsyncWebServerRequest* request) {
        request->redirect("/");
    });

    server->begin();
    serverRunning = true;

    Serial.print("[WEB] Dashboard: http://");
    Serial.println((WiFi.getMode() & WIFI_AP) ? WiFi.softAPIP() : WiFi.localIP());
}

void stop() {
    if (server) {
        server->end();
        serverRunning = false;
    }
}

bool isRunning() {
    return serverRunning;
}

} // namespace WebServerManager
