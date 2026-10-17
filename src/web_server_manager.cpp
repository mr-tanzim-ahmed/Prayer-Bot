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
#include <sys/time.h>
#include <math.h>
#include <new>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern volatile bool g_locationRefreshRequested;
extern volatile bool g_weatherRefreshRequested;
extern volatile bool g_ntpSyncRequested;
extern PomodoroState g_pomodoro;
extern SemaphoreHandle_t g_settingsMutex;
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
        } else if (!doc["clearOwmKey"].isUnbound() &&
                   !doc["clearOwmKey"].is<bool>()) {
            error = "Clear API key must be true or false.";
        } else if (doc["clearOwmKey"].as<bool>() &&
                   doc["owmKey"].is<const char*>() &&
                   doc["owmKey"].as<const char*>()[0] != '\0') {
            error = "Choose either a new API key or clear the saved key.";
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

    String formatTime(uint8_t hour, uint8_t minute) {
        char text[6];
        snprintf(text, sizeof(text), "%02u:%02u", hour, minute);
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
            "sunsetOff", "owmKey", "clearOwmKey"
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

        if (!g_settingsMutex ||
            xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
            request->send(503, "application/json", "{\"error\":\"Settings are busy; try again shortly.\"}");
            return;
        }

        const Settings previousSettings = *settingsRef;
        Settings candidate = previousSettings;
        if (doc["azanOn"].is<bool>())
            candidate.azanEnabled = doc["azanOn"].as<bool>();
        if (doc["volume"].is<int>())
            candidate.volume = doc["volume"].as<uint8_t>();
        if (doc["owmKey"].is<const char*>()) {
            const char* apiKey = doc["owmKey"].as<const char*>();
            if (apiKey[0] != '\0')
                strlcpy(candidate.owmApiKey, apiKey, sizeof(candidate.owmApiKey));
        }
        if (doc["clearOwmKey"] | false) candidate.owmApiKey[0] = '\0';
        if (doc["city"].is<const char*>())
            strlcpy(candidate.cityName, doc["city"].as<const char*>(), sizeof(candidate.cityName));
        if (doc["timezone"].is<const char*>())
            strlcpy(candidate.timezone, doc["timezone"].as<const char*>(), sizeof(candidate.timezone));
        if (isNumber(doc["lat"])) candidate.latitude = doc["lat"].as<float>();
        if (isNumber(doc["lon"])) candidate.longitude = doc["lon"].as<float>();
        if (isNumber(doc["utcOff"])) candidate.utcOffset = doc["utcOff"].as<float>();
        if (doc["calcMethod"].is<int>()) candidate.calcMethod = doc["calcMethod"].as<uint8_t>();
        if (doc["asrSchool"].is<int>()) candidate.asrSchool = doc["asrSchool"].as<uint8_t>();
        if (doc["hijriOffset"].is<int>()) candidate.hijriOffset = doc["hijriOffset"].as<int8_t>();
        if (doc["focusMin"].is<int>()) candidate.focusMinutes = doc["focusMin"].as<uint16_t>();
        if (doc["shortBrk"].is<int>()) candidate.shortBreakMinutes = doc["shortBrk"].as<uint16_t>();
        if (doc["longBrk"].is<int>()) candidate.longBreakMinutes = doc["longBrk"].as<uint16_t>();
        if (doc["pomCycles"].is<int>()) candidate.pomodoroCycles = doc["pomCycles"].as<uint8_t>();
        if (doc["sunriseOff"].is<int>()) candidate.sunriseOffset = doc["sunriseOff"].as<uint8_t>();
        if (doc["zawalOff"].is<int>()) candidate.zawalOffset = doc["zawalOff"].as<uint8_t>();
        if (doc["sunsetOff"].is<int>()) candidate.sunsetOffset = doc["sunsetOff"].as<uint8_t>();

        if (!StorageManager::saveSettings(candidate)) {
            xSemaphoreGive(g_settingsMutex);
            request->send(500, "application/json", "{\"error\":\"Could not save settings to device storage.\"}");
            return;
        }

        *settingsRef = candidate;
        xSemaphoreGive(g_settingsMutex);
        const bool locationChanged =
            candidate.latitude != previousSettings.latitude ||
            candidate.longitude != previousSettings.longitude ||
            candidate.calcMethod != previousSettings.calcMethod ||
            candidate.asrSchool != previousSettings.asrSchool ||
            strcmp(candidate.cityName, previousSettings.cityName) != 0;
        const bool timezoneChanged =
            candidate.utcOffset != previousSettings.utcOffset ||
            strcmp(candidate.timezone, previousSettings.timezone) != 0;
        const bool weatherConfigChanged =
            strcmp(candidate.owmApiKey, previousSettings.owmApiKey) != 0;

        g_webVolumeRequest = candidate.volume;
        PomodoroManager::updateSettings(g_pomodoro, candidate);
        CalendarManager::setHijriOffset(candidate.hijriOffset);
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
            request->_tempObject = nullptr;
            if (total > MAX_REQUEST_BODY_BYTES) {
                request->send(413, "application/json", "{\"error\":\"Request body is too large.\"}");
                return nullptr;
            }
            String* body = new (std::nothrow) String();
            if (!body || !body->reserve(total)) {
                delete body;
                request->send(503, "application/json", "{\"error\":\"Not enough memory to receive the request.\"}");
                return nullptr;
            }
            request->_tempObject = body;
        }
        String* body = static_cast<String*>(request->_tempObject);
        if (!body || index != body->length() || index > total || len > total - index) {
            delete body;
            request->_tempObject = nullptr;
            request->send(400, "application/json", "{\"error\":\"Incomplete or invalid request body.\"}");
            return nullptr;
        }
        if (!body->concat(reinterpret_cast<const char*>(data), len)) {
            delete body;
            request->_tempObject = nullptr;
            request->send(503, "application/json", "{\"error\":\"Could not buffer the request body.\"}");
            return nullptr;
        }
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
        if (!g_settingsMutex ||
            xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
            request->send(503, "application/json", "{\"error\":\"Settings are busy; try again shortly.\"}");
            return;
        }
        const Settings snapshot = *settingsRef;
        xSemaphoreGive(g_settingsMutex);

        JsonDocument doc;
        doc["azanOn"] = snapshot.azanEnabled;
        doc["volume"] = snapshot.volume;
        doc["city"] = snapshot.cityName;
        doc["lat"] = snapshot.latitude;
        doc["lon"] = snapshot.longitude;
        doc["timezone"] = snapshot.timezone;
        doc["utcOff"] = snapshot.utcOffset;
        doc["calcMethod"] = snapshot.calcMethod;
        doc["asrSchool"] = snapshot.asrSchool;
        doc["hijriOffset"] = snapshot.hijriOffset;
        doc["focusMin"] = snapshot.focusMinutes;
        doc["shortBrk"] = snapshot.shortBreakMinutes;
        doc["longBrk"] = snapshot.longBreakMinutes;
        doc["pomCycles"] = snapshot.pomodoroCycles;
        doc["sunriseOff"] = snapshot.sunriseOffset;
        doc["zawalOff"] = snapshot.zawalOffset;
        doc["sunsetOff"] = snapshot.sunsetOffset;
        doc["owmKeyConfigured"] = snapshot.owmApiKey[0] != '\0';
        doc["setupApSsid"] = WifiManager::getSetupAPSSID();
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

    server->on("/api/time", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String* body = appendRequestBody(request, data, len, index, total);
            if (!body) return;

            JsonDocument doc;
            if (deserializeJson(doc, *body) || !doc.is<JsonObject>() ||
                doc.size() != 1 || !doc["epoch"].is<uint32_t>()) {
                delete body;
                request->send(400, "application/json", "{\"error\":\"A valid Unix epoch in seconds is required.\"}");
                return;
            }
            delete body;

            const uint32_t epoch = doc["epoch"].as<uint32_t>();
            if (epoch < 1577836800UL || epoch > 4102444799UL) {
                request->send(400, "application/json", "{\"error\":\"Device time must be between 2020 and 2099.\"}");
                return;
            }
            if (!g_settingsMutex ||
                xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
                request->send(503, "application/json", "{\"error\":\"Settings are busy; try again shortly.\"}");
                return;
            }
            const float utcOffset = settingsRef->utcOffset;
            xSemaphoreGive(g_settingsMutex);

            configTime((long)(utcOffset * 3600), 0, NTP_SERVER);
            timeval deviceTime = {};
            deviceTime.tv_sec = static_cast<time_t>(epoch);
            if (settimeofday(&deviceTime, nullptr) != 0) {
                request->send(500, "application/json", "{\"error\":\"Could not set the device clock.\"}");
                return;
            }

            Serial.println("[TIME] Initialized device clock from dashboard local machine; NTP will correct it when online.");
            request->send(200, "application/json", "{\"ok\":true,\"source\":\"dashboard-local-machine\"}");
        });

    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        JsonObject network = doc["network"].to<JsonObject>();
        const bool stationConnected = WiFi.status() == WL_CONNECTED;
        const bool connecting = WifiManager::isConnecting();
        network["connected"] = stationConnected;
        network["mode"] = (WiFi.getMode() & WIFI_AP)
            ? (stationConnected ? "access-point+station" : "access-point")
            : "station";
        network["setupRequired"] =
            (WiFi.getMode() & WIFI_AP) && !stationConnected;
        network["connecting"] = connecting;
        network["authFailed"] =
            !stationConnected && !connecting && WiFi.status() == WL_CONNECT_FAILED;
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

        JsonObject prohibited = doc["prohibited"].to<JsonObject>();
        prohibited["ready"] = g_prayerReady;
        JsonObject afterSunrise = prohibited["afterSunrise"].to<JsonObject>();
        afterSunrise["start"] = formatTime(g_prohibited.afterSunrise.startHour,
                                           g_prohibited.afterSunrise.startMinute);
        afterSunrise["end"] = formatTime(g_prohibited.afterSunrise.endHour,
                                         g_prohibited.afterSunrise.endMinute);
        afterSunrise["active"] = g_prohibited.afterSunrise.active;
        JsonObject atZawal = prohibited["atZawal"].to<JsonObject>();
        atZawal["start"] = formatTime(g_prohibited.atZawal.startHour,
                                      g_prohibited.atZawal.startMinute);
        atZawal["end"] = formatTime(g_prohibited.atZawal.endHour,
                                    g_prohibited.atZawal.endMinute);
        atZawal["active"] = g_prohibited.atZawal.active;
        JsonObject beforeSunset = prohibited["beforeSunset"].to<JsonObject>();
        beforeSunset["start"] = formatTime(g_prohibited.beforeSunset.startHour,
                                           g_prohibited.beforeSunset.startMinute);
        beforeSunset["end"] = formatTime(g_prohibited.beforeSunset.endHour,
                                         g_prohibited.beforeSunset.endMinute);
        beforeSunset["active"] = g_prohibited.beforeSunset.active;

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

    server->on("/api/setup-ap", HTTP_POST,
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
                request->send(400, "application/json", "{\"error\":\"Hotspot settings must be a JSON object.\"}");
                return;
            }
            static const char* const setupAPKeys[] = {"ssid", "pass"};
            if (!hasOnlyKeys(doc.as<JsonObjectConst>(), setupAPKeys,
                             sizeof(setupAPKeys) / sizeof(setupAPKeys[0]))) {
                request->send(400, "application/json", "{\"error\":\"Hotspot settings contain an unknown field.\"}");
                return;
            }
            if (!validString(doc["ssid"], 32) || !validString(doc["pass"], 63, true)) {
                request->send(400, "application/json", "{\"error\":\"Enter a hotspot name (up to 32 characters) and a password of 8 to 63 characters, or leave the password blank to keep it.\"}");
                return;
            }
            String ssid = doc["ssid"].as<String>();
            String password = doc["pass"].as<String>();
            if (!password.isEmpty() && password.length() < 8) {
                request->send(400, "application/json", "{\"error\":\"A hotspot password must contain at least 8 characters.\"}");
                return;
            }
            if (!WifiManager::saveSetupAPSettings(ssid, password)) {
                request->send(500, "application/json", "{\"error\":\"Could not save hotspot settings to device storage.\"}");
                return;
            }
            request->send(200, "application/json", "{\"status\":\"saved\",\"message\":\"Hotspot settings saved. The hotspot will restart shortly; reconnect using the new name and password.\"}");
        });

    server->on("/api/wifi/scan", HTTP_POST,
        [](AsyncWebServerRequest* request) {
            WiFi.mode(WIFI_AP_STA);
            WiFi.setSleep(false);
            if (WiFi.scanComplete() == WIFI_SCAN_RUNNING) {
                request->send(200, "application/json", "{\"status\":\"scanning\"}");
                return;
            }

            if (WiFi.scanNetworks(true, false) == WIFI_SCAN_FAILED) {
                request->send(500, "application/json", "{\"error\":\"Could not start Wi-Fi scan.\"}");
                return;
            }
            request->send(200, "application/json", "{\"status\":\"scanning\"}");
        });

    server->on("/api/wifi/scan", HTTP_GET,
        [](AsyncWebServerRequest* request) {
            const int16_t scanResult = WiFi.scanComplete();
            if (scanResult == WIFI_SCAN_RUNNING) {
                request->send(200, "application/json", "{\"status\":\"scanning\"}");
                return;
            }
            if (scanResult == WIFI_SCAN_FAILED) {
                request->send(500, "application/json", "{\"error\":\"Wi-Fi scan failed.\"}");
                return;
            }

            JsonDocument doc;
            doc["status"] = "complete";
            JsonArray networks = doc["networks"].to<JsonArray>();
            for (int16_t i = 0; i < scanResult; ++i) {
                String ssid = WiFi.SSID(i);
                if (ssid.isEmpty()) continue;
                JsonObject network = networks.add<JsonObject>();
                network["ssid"] = ssid;
                network["rssi"] = WiFi.RSSI(i);
                network["secured"] = WiFi.encryptionType(i) != WIFI_AUTH_OPEN;
            }

            String response;
            serializeJson(doc, response);
            WiFi.scanDelete();
            request->send(200, "application/json", response);
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
            if (!WifiManager::saveCredentials(ssid, password)) {
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
