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
#include "voice_manager.h"
#include "azan_manager.h"
#include "https_server_manager.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <esp_heap_caps.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <new>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern volatile bool g_locationRefreshRequested;
extern volatile bool g_weatherRefreshRequested;
extern volatile bool g_ntpSyncRequested;
extern PomodoroState g_pomodoro;
extern SemaphoreHandle_t g_settingsMutex;
extern volatile int16_t g_webVolumeRequest;
extern bool g_timesynced;
extern bool g_prayerReady;

// ============================================================
//  WEB SERVER MANAGER IMPLEMENTATION
//  Serves the local device dashboard and its JSON API.
// ============================================================

namespace {
    AsyncWebServer* server = nullptr;
    bool serverRunning = false;
    Settings* settingsRef = nullptr;
    constexpr size_t MAX_REQUEST_BODY_BYTES = 2048;
    constexpr size_t MAX_DASHBOARD_AUDIO_BYTES = 256 * 1024;

    struct DashboardAudioUpload {
        uint8_t* data;
        size_t total;
        size_t received;
        char mimeType[32];
    };

    void freeDashboardAudioUpload(DashboardAudioUpload* upload) {
        if (!upload) return;
        if (upload->data) heap_caps_free(upload->data);
        delete upload;
    }

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
            "sunsetOff"
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
        g_webVolumeRequest = candidate.volume;
        PomodoroManager::updateSettings(g_pomodoro, candidate);
        CalendarManager::setHijriOffset(candidate.hijriOffset);
        if (timezoneChanged) g_ntpSyncRequested = true;
        if (locationChanged) {
            g_locationRefreshRequested = true;
            g_weatherRefreshRequested = true;
        }

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
extern volatile bool g_webAzanStartRequested;

namespace WebServerManager {

void init(Settings& settings) {
    HttpsServerManager::stop();
    if (server) {
        server->end();
        delete server;
    }

    settingsRef = &settings;
    server = new AsyncWebServer(WEB_SERVER_PORT);

    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", DASHBOARD_HTML);
    });
    static const char* const captivePortalPaths[] = {
        "/generate_204", "/gen_204", "/hotspot-detect.html",
        "/library/test/success.html", "/connecttest.txt",
        "/ncsi.txt", "/redirect", "/canonical.html", "/success.txt",
        "/fwlink"
    };
    for (const char* path : captivePortalPaths) {
        server->on(path, HTTP_GET, [](AsyncWebServerRequest* request) {
            request->send(200, "text/html", DASHBOARD_HTML);
        });
    }

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
            g_timesynced = true;
            g_locationRefreshRequested = true;
            request->send(200, "application/json", "{\"ok\":true,\"source\":\"dashboard-local-machine\"}");
        });

    auto handlePrayerRefresh = [](AsyncWebServerRequest* request) {
        if (settingsRef && WifiManager::isConnected()) {
            if (PrayerManager::refresh(*settingsRef)) {
                PrayerManager::getTodayPrayers(g_todayPrayers);
                PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, *settingsRef);
                CalendarManager::getHijriDate(g_hijriDate);
                CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
            }
        } else {
            g_locationRefreshRequested = true;
        }

        DailyPrayers today;
        PrayerManager::getTodayPrayers(today);
        ProhibitedTimes prob = g_prohibited;
        if (prob.afterSunrise.startHour == 0 && prob.afterSunrise.startMinute == 0 && settingsRef) {
            PrayerManager::updateNextPrayer(g_nextPrayer, prob, *settingsRef);
        }

        JsonDocument doc;
        doc["status"] = "ok";
        doc["liveSynced"] = PrayerManager::isLiveSynced();
        JsonObject prayers = doc["prayers"].to<JsonObject>();
        prayers["ready"] = true;
        prayers["liveSynced"] = PrayerManager::isLiveSynced();
        prayers["fajr"] = formatTime(today.fajr);
        prayers["sunrise"] = formatTime(today.sunrise);
        prayers["dhuhr"] = formatTime(today.dhuhr);
        prayers["asr"] = formatTime(today.asr);
        prayers["maghrib"] = formatTime(today.maghrib);
        prayers["isha"] = formatTime(today.isha);

        JsonObject prohibited = doc["prohibited"].to<JsonObject>();
        prohibited["ready"] = true;
        JsonObject afterSunrise = prohibited["afterSunrise"].to<JsonObject>();
        afterSunrise["start"] = formatTime(prob.afterSunrise.startHour, prob.afterSunrise.startMinute);
        afterSunrise["end"] = formatTime(prob.afterSunrise.endHour, prob.afterSunrise.endMinute);
        afterSunrise["active"] = prob.afterSunrise.active;

        JsonObject atZawal = prohibited["atZawal"].to<JsonObject>();
        atZawal["start"] = formatTime(prob.atZawal.startHour, prob.atZawal.startMinute);
        atZawal["end"] = formatTime(prob.atZawal.endHour, prob.atZawal.endMinute);
        atZawal["active"] = prob.atZawal.active;

        JsonObject beforeSunset = prohibited["beforeSunset"].to<JsonObject>();
        beforeSunset["start"] = formatTime(prob.beforeSunset.startHour, prob.beforeSunset.startMinute);
        beforeSunset["end"] = formatTime(prob.beforeSunset.endHour, prob.beforeSunset.endMinute);
        beforeSunset["active"] = prob.beforeSunset.active;

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    };

    server->on("/api/prayer/refresh", HTTP_POST, handlePrayerRefresh);
    server->on("/api/prayer/refresh", HTTP_GET, handlePrayerRefresh);

    server->on("/api/status", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!PrayerManager::isLiveSynced() && WifiManager::isConnected()) {
            g_locationRefreshRequested = true;
        }
        JsonDocument doc;
        JsonObject network = doc["network"].to<JsonObject>();
        const bool stationConnected = WifiManager::isConnected();
        const bool connecting = WifiManager::connectionInProgress();
        network["connected"] = stationConnected;
        network["mode"] = (WiFi.getMode() & WIFI_AP)
            ? (stationConnected ? "access-point+station" : "access-point")
            : "station";
        network["setupRequired"] =
            (WiFi.getMode() & WIFI_AP) && !stationConnected;
        network["connecting"] = connecting;
        network["restarting"] = WifiManager::shouldRestartAfterConnection();
        network["connectionFailed"] = WifiManager::didConnectionAttemptFail();
        network["authFailed"] =
            !stationConnected && !connecting && WiFi.status() == WL_CONNECT_FAILED;
        network["ip"] = WifiManager::getIP();
        network["homeIp"] = stationConnected
            ? WiFi.localIP().toString()
            : "";

        DailyPrayers today;
        PrayerManager::getTodayPrayers(today);
        ProhibitedTimes prob = g_prohibited;
        if (prob.afterSunrise.startHour == 0 && prob.afterSunrise.startMinute == 0 && settingsRef) {
            PrayerManager::updateNextPrayer(g_nextPrayer, prob, *settingsRef);
        }

        JsonObject prayers = doc["prayers"].to<JsonObject>();
        prayers["ready"] = true;
        prayers["liveSynced"] = PrayerManager::isLiveSynced();
        prayers["fajr"] = formatTime(today.fajr);
        prayers["sunrise"] = formatTime(today.sunrise);
        prayers["dhuhr"] = formatTime(today.dhuhr);
        prayers["asr"] = formatTime(today.asr);
        prayers["maghrib"] = formatTime(today.maghrib);
        prayers["isha"] = formatTime(today.isha);

        JsonObject prohibited = doc["prohibited"].to<JsonObject>();
        prohibited["ready"] = true;
        JsonObject afterSunrise = prohibited["afterSunrise"].to<JsonObject>();
        afterSunrise["start"] = formatTime(prob.afterSunrise.startHour, prob.afterSunrise.startMinute);
        afterSunrise["end"] = formatTime(prob.afterSunrise.endHour, prob.afterSunrise.endMinute);
        afterSunrise["active"] = prob.afterSunrise.active;
        JsonObject atZawal = prohibited["atZawal"].to<JsonObject>();
        atZawal["start"] = formatTime(prob.atZawal.startHour, prob.atZawal.startMinute);
        atZawal["end"] = formatTime(prob.atZawal.endHour, prob.atZawal.endMinute);
        atZawal["active"] = prob.atZawal.active;
        JsonObject beforeSunset = prohibited["beforeSunset"].to<JsonObject>();
        beforeSunset["start"] = formatTime(prob.beforeSunset.startHour, prob.beforeSunset.startMinute);
        beforeSunset["end"] = formatTime(prob.beforeSunset.endHour, prob.beforeSunset.endMinute);
        beforeSunset["active"] = prob.beforeSunset.active;

        JsonObject next = doc["nextPrayer"].to<JsonObject>();
        next["name"] = g_prayerReady ? PrayerManager::getPrayerNameStr(g_nextPrayer.name) : "";
        next["time"] = formatTime(g_nextPrayer.startTime);
        next["countdown"] = formatDuration(g_nextPrayer.countdownSeconds);
        JsonObject quran = doc["quran"].to<JsonObject>();
        const uint8_t curSurah = VoiceManager::getPlayingSurah();
        quran["surah"] = VoiceManager::getSurahName(curSurah);
        quran["surahNumber"] = curSurah;
        quran["status"] = VoiceManager::getStatusText();
        quran["playing"] = AzanManager::isQuranPlaying();
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
        weather["airQualityValid"] = g_weatherReady && g_weather.airQualityValid;
        weather["city"] = g_weather.cityName;
        weather["temperatureC"] = g_weather.temperatureC;
        weather["humidity"] = g_weather.humidity;
        weather["aqi"] = g_weather.airQualityIndex;
        weather["pm25"] = g_weather.pm25;
        weather["pm10"] = g_weather.pm10;
        weather["co"] = g_weather.carbonMonoxide;
        weather["no"] = g_weather.nitrogenMonoxide;
        weather["no2"] = g_weather.nitrogenDioxide;
        weather["o3"] = g_weather.ozone;
        weather["so2"] = g_weather.sulphurDioxide;
        weather["nh3"] = g_weather.ammonia;
        weather["condition"] = g_weather.weatherDescription;

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
                "screen", "pomodoro", "dhikr", "stopAzan", "playAzan",
                "quranAction", "quranSurah"
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
            JsonVariant playAzanValue = doc["playAzan"];
            JsonVariant quranActionValue = doc["quranAction"];
            JsonVariant quranSurahValue = doc["quranSurah"];
            if ((!screenValue.isUnbound() && !screenValue.is<int>()) ||
                (!pomodoroValue.isUnbound() && !pomodoroValue.is<const char*>()) ||
                (!dhikrValue.isUnbound() && !dhikrValue.is<const char*>()) ||
                (!stopAzanValue.isUnbound() && !stopAzanValue.is<bool>()) ||
                (!playAzanValue.isUnbound() && !playAzanValue.is<bool>()) ||
                (!quranActionValue.isUnbound() && !quranActionValue.is<const char*>()) ||
                (!quranSurahValue.isUnbound() && !quranSurahValue.is<int>())) {
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
            const bool playAzan = !playAzanValue.isUnbound() && playAzanValue.as<bool>();
            const bool hasQuranAction = !quranActionValue.isUnbound();
            const bool hasCommand = screen >= 0 || pomodoroCommand != 0 ||
                                    countDhikr || stopAzan || playAzan ||
                                    hasQuranAction;
            if (!hasCommand) {
                request->send(400, "application/json", "{\"error\":\"No supported device command was provided.\"}");
                return;
            }

            if (screen >= 0) g_webScreenRequest = (int8_t)screen;
            if (pomodoroCommand != 0) g_webPomodoroRequest = pomodoroCommand;
            if (countDhikr) g_webDhikrRequest = 1;
            if (stopAzan) g_azanStopRequested = true;
            if (playAzan) g_webAzanStartRequested = true;
            if (hasQuranAction) {
                const char* qAct = quranActionValue.as<const char*>();
                if (strcmp(qAct, "play") == 0) {
                    int sNum = quranSurahValue.is<int>() ? quranSurahValue.as<int>() : VoiceManager::getPlayingSurah();
                    VoiceManager::playSurah(sNum);
                    g_webScreenRequest = SYS_SCREEN_QURAN;
                } else if (strcmp(qAct, "stop") == 0) {
                    VoiceManager::stopPlayback();
                } else if (strcmp(qAct, "next") == 0) {
                    VoiceManager::nextSurah();
                } else if (strcmp(qAct, "prev") == 0) {
                    VoiceManager::prevSurah();
                }
            }
            request->send(200, "application/json", "{\"status\":\"queued\"}");
        });

    server->on("/api/quran/recognize", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String* body = appendRequestBody(request, data, len, index, total);
            if (!body) return;
            JsonDocument doc;
            if (deserializeJson(doc, *body) || !doc.is<JsonObject>() ||
                doc.size() != 1 || !doc["transcript"].is<const char*>()) {
                delete body;
                request->send(400, "application/json", "{\"error\":\"Send one transcript string to recognize.\"}");
                return;
            }
            const String transcript = doc["transcript"].as<String>();
            delete body;
            if (transcript.isEmpty() || transcript.length() > 120) {
                request->send(400, "application/json", "{\"error\":\"Transcript must contain 1 to 120 characters.\"}");
                return;
            }
            if (WiFi.status() != WL_CONNECTED) {
                request->send(503, "application/json", "{\"error\":\"Not connected to router Wi-Fi. Connect Wi-Fi for Gemini voice recognition.\"}");
                return;
            }
            uint32_t requestId = 0;
            if (!VoiceManager::queueDashboardRecognition(transcript.c_str(),
                                                         requestId)) {
                request->send(503, "application/json", "{\"error\":\"Voice recognition is busy, unavailable, or could not be queued. Try again shortly.\"}");
                return;
            }
            JsonDocument response;
            response["requestId"] = requestId;
            String json;
            serializeJson(response, json);
            request->send(202, "application/json", json);
        });

    server->on("/api/quran/recognize-audio", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len,
           size_t index, size_t total) {
            if (index == 0) {
                auto* upload = new (std::nothrow) DashboardAudioUpload{};
                if (!upload || total == 0 || total > MAX_DASHBOARD_AUDIO_BYTES) {
                    delete upload;
                    request->send(413, "application/json",
                                  "{\"error\":\"Audio must be shorter than 256 KB.\"}");
                    return;
                }
                String mimeType = request->contentType();
                const int parameters = mimeType.indexOf(';');
                if (parameters >= 0) mimeType.remove(parameters);
                mimeType.trim();
                if (mimeType != "audio/mp4" && mimeType != "audio/webm" &&
                    mimeType != "audio/wav" && mimeType != "audio/aac") {
                    delete upload;
                    request->send(415, "application/json",
                                  "{\"error\":\"Use Safari audio/mp4 or a supported audio recording format.\"}");
                    return;
                }
                upload->data = static_cast<uint8_t*>(
                    heap_caps_malloc(total, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
                if (!upload->data) {
                    delete upload;
                    request->send(503, "application/json",
                                  "{\"error\":\"Not enough device memory for this audio recording.\"}");
                    return;
                }
                upload->total = total;
                strlcpy(upload->mimeType, mimeType.c_str(),
                        sizeof(upload->mimeType));
                request->_tempObject = upload;
                request->onDisconnect([request]() {
                    auto* pending = static_cast<DashboardAudioUpload*>(
                        request->_tempObject);
                    request->_tempObject = nullptr;
                    freeDashboardAudioUpload(pending);
                });
            }

            auto* upload = static_cast<DashboardAudioUpload*>(request->_tempObject);
            if (!upload || index != upload->received || len > upload->total - upload->received ||
                total != upload->total) {
                request->_tempObject = nullptr;
                freeDashboardAudioUpload(upload);
                request->send(400, "application/json",
                              "{\"error\":\"Audio upload was incomplete or invalid.\"}");
                return;
            }
            memcpy(upload->data + upload->received, data, len);
            upload->received += len;
            if (upload->received != upload->total) return;

            request->_tempObject = nullptr;
            uint32_t requestId = 0;
            if (!VoiceManager::queueDashboardAudioRecognition(
                    upload->data, upload->total, upload->mimeType, requestId)) {
                freeDashboardAudioUpload(upload);
                request->send(503, "application/json",
                              "{\"error\":\"Gemini audio recognition is busy or unavailable. Try again shortly.\"}");
                return;
            }
            upload->data = nullptr;
            freeDashboardAudioUpload(upload);
            JsonDocument response;
            response["requestId"] = requestId;
            String json;
            serializeJson(response, json);
            request->send(202, "application/json", json);
        });

    server->on("/api/quran/recognize", HTTP_GET, [](AsyncWebServerRequest* request) {
        if (!request->hasParam("id")) {
            request->send(400, "application/json", "{\"error\":\"A recognition request ID is required.\"}");
            return;
        }
        const String idText = request->getParam("id")->value();
        char* end = nullptr;
        const unsigned long parsedId = strtoul(idText.c_str(), &end, 10);
        if (idText.isEmpty() || !end || *end != '\0' || parsedId == 0 ||
            parsedId > UINT32_MAX) {
            request->send(400, "application/json", "{\"error\":\"Recognition request ID is invalid.\"}");
            return;
        }
        uint8_t surahNumber = 0;
        const char* status = VoiceManager::getDashboardRecognition(
            static_cast<uint32_t>(parsedId), surahNumber);
        JsonDocument response;
        response["status"] = status;
        if (strcmp(status, "done") == 0) {
            if (surahNumber == TOTAL_SURAHS + 1) {
                response["intent"] = "azan";
            } else if (surahNumber >= 1 && surahNumber <= TOTAL_SURAHS) {
                response["intent"] = "surah";
                response["surahNumber"] = surahNumber;
                response["surah"] = VoiceManager::getSurahName(surahNumber);
            } else {
                response["status"] = "error";
                response["error"] = "Gemini did not recognize a supported command.";
            }
        } else if (strcmp(status, "error") == 0) {
            response["error"] =
                strcmp(VoiceManager::getStatusText(), "Gemini key not configured") == 0
                    ? "Gemini API key is not configured on Prayer-Bot."
                    : "Gemini could not identify a Surah. Speak its name clearly and try again.";
        }
        String json;
        serializeJson(response, json);
        request->send(200, "application/json", json);
    });

    server->on("/api/quran", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        const uint8_t cur = VoiceManager::getPlayingSurah();
        doc["surah"] = cur;
        doc["name"] = VoiceManager::getSurahName(cur);
        doc["playing"] = AzanManager::isQuranPlaying();
        doc["status"] = VoiceManager::getStatusText();
        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    server->on("/api/quran", HTTP_POST,
        [](AsyncWebServerRequest*) {},
        nullptr,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String* body = appendRequestBody(request, data, len, index, total);
            if (!body) return;
            JsonDocument doc;
            if (deserializeJson(doc, *body) || !doc.is<JsonObject>()) {
                delete body;
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }
            delete body;
            static const char* const quranKeys[] = {"action", "surah"};
            if (!hasOnlyKeys(doc.as<JsonObjectConst>(), quranKeys,
                             sizeof(quranKeys) / sizeof(quranKeys[0]))) {
                request->send(400, "application/json", "{\"error\":\"Quran request contains an unknown field.\"}");
                return;
            }
            if (!doc["action"].is<const char*>()) {
                request->send(400, "application/json", "{\"error\":\"A Quran action is required.\"}");
                return;
            }
            const char* action = doc["action"].as<const char*>();
            int surah = doc["surah"] | 0;
            const bool needsSurah = strcmp(action, "play") == 0 ||
                                    strcmp(action, "select") == 0;
            if ((needsSurah && (doc.size() != 2 || !doc["surah"].is<int>())) ||
                (!needsSurah && doc.size() != 1)) {
                request->send(400, "application/json", "{\"error\":\"Only play and select actions accept a Surah number.\"}");
                return;
            }
            const bool needsInternet = strcmp(action, "play") == 0 ||
                                       strcmp(action, "next") == 0 ||
                                       strcmp(action, "prev") == 0;
            if (needsInternet && WiFi.status() != WL_CONNECTED) {
                request->send(503, "application/json", "{\"error\":\"Not connected to router Wi-Fi. Connect Wi-Fi to play Quran audio.\"}");
                return;
            }
            if (strcmp(action, "play") == 0) {
                if (!doc["surah"].is<int>() || surah < 1 || surah > TOTAL_SURAHS) {
                    request->send(400, "application/json", "{\"error\":\"Choose a Surah number from 1 to 114.\"}");
                    return;
                }
                g_webScreenRequest = SYS_SCREEN_QURAN;
                if (!VoiceManager::playSurah(surah)) {
                    request->send(503, "application/json", "{\"error\":\"Could not queue Quran playback. Check Wi-Fi and audio output.\"}");
                    return;
                }
                request->send(200, "application/json", "{\"status\":\"queued\"}");
            } else if (strcmp(action, "stop") == 0) {
                VoiceManager::stopPlayback();
                request->send(200, "application/json", "{\"status\":\"stopped\"}");
            } else if (strcmp(action, "next") == 0) {
                g_webScreenRequest = SYS_SCREEN_QURAN;
                VoiceManager::nextSurah();
                request->send(200, "application/json", "{\"status\":\"queued\"}");
            } else if (strcmp(action, "prev") == 0) {
                g_webScreenRequest = SYS_SCREEN_QURAN;
                VoiceManager::prevSurah();
                request->send(200, "application/json", "{\"status\":\"queued\"}");
            } else if (strcmp(action, "select") == 0) {
                if (!doc["surah"].is<int>() || surah < 1 || surah > TOTAL_SURAHS) {
                    request->send(400, "application/json", "{\"error\":\"Choose a Surah number from 1 to 114.\"}");
                    return;
                }
                VoiceManager::selectSurah(surah);
                g_webScreenRequest = SYS_SCREEN_QURAN;
                request->send(200, "application/json", "{\"status\":\"selected\"}");
            } else {
                request->send(400, "application/json", "{\"error\":\"Unknown action\"}");
            }
        });

    server->on("/api/wifi/scan", HTTP_POST,
        [](AsyncWebServerRequest* request) {
            if (WifiManager::connectionInProgress()) {
                WifiManager::abortConnection();
                delay(100);
            }
            WiFi.setSleep(false);
            const int16_t running = WiFi.scanComplete();
            if (running == WIFI_SCAN_RUNNING) {
                // Scan already in progress — just report scanning
                request->send(200, "application/json", "{\"status\":\"scanning\"}");
                return;
            }
            // Delete any stale results before starting a fresh async scan
            if (running >= 0) WiFi.scanDelete();
            // scanNetworks(async=true) returns WIFI_SCAN_RUNNING on success
            const int16_t ret = WiFi.scanNetworks(/*async*/true, /*show_hidden*/false);
            if (ret == WIFI_SCAN_FAILED) {
                request->send(500, "application/json", "{\"error\":\"Could not start Wi-Fi scan.\"}");
                return;
            }
            request->send(200, "application/json", "{\"status\":\"scanning\"}");
        });

    server->on("/api/wifi/scan", HTTP_GET,
        [](AsyncWebServerRequest* request) {
            const int16_t scanResult = WiFi.scanComplete();
            if (scanResult == WIFI_SCAN_RUNNING) {
                // Async scan still in progress
                request->send(200, "application/json", "{\"status\":\"scanning\"}");
                return;
            }
            if (scanResult == WIFI_SCAN_FAILED) {
                // No scan has been started yet — treat as pending rather than error
                request->send(200, "application/json", "{\"status\":\"scanning\"}");
                return;
            }
            // scanResult >= 0 means scan complete with that many networks found
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

            if (!doc["ssid"].is<const char*>() ||
                !doc["pass"].is<const char*>() ||
                !validString(doc["ssid"], 32) ||
                !validString(doc["pass"], 63, true) ||
                (doc["pass"].as<const char*>()[0] != '\0' &&
                 strlen(doc["pass"].as<const char*>()) < 8)) {
                request->send(400, "application/json", "{\"error\":\"Enter an SSID (up to 32 characters) and an empty or 8-63 character password.\"}");
                return;
            }
            String ssid = doc["ssid"].as<String>();
            String password = doc["pass"].as<String>();
            if (!WifiManager::connectToNetwork(ssid, password)) {
                request->send(409, "application/json", "{\"error\":\"A new Wi-Fi credential test is already in progress, or the connection could not start.\"}");
                return;
            }
            WifiManager::restartAfterConnection();
            request->send(200, "application/json", "{\"status\":\"connecting\",\"message\":\"Testing the router connection and DHCP address. Credentials will be saved only after the router lease is valid, then the device will restart.\"}");
        });

    server->onNotFound([](AsyncWebServerRequest* request) {
        request->redirect("/");
    });

    server->begin();
    serverRunning = true;
    const bool mdnsStarted = WiFi.status() == WL_CONNECTED &&
                             MDNS.begin("prayer-bot");
    if (mdnsStarted) {
        MDNS.addService("http", "tcp", WEB_SERVER_PORT);
        Serial.println("[WEB] Dashboard hostname: prayer-bot.local");
    }
    if (!HttpsServerManager::init()) {
        Serial.println("[WEB] HTTPS could not start; the HTTP dashboard remains available.");
    } else if (mdnsStarted) {
        MDNS.addService("https", "tcp", WEB_SERVER_HTTPS_PORT);
    }
    if (!mdnsStarted && WiFi.status() == WL_CONNECTED) {
        Serial.println("[WEB] Could not start mDNS; use the device IP address.");
    }

    Serial.print("[WEB] Dashboard: http://");
    Serial.println((WiFi.getMode() & WIFI_AP) ? WiFi.softAPIP() : WiFi.localIP());
    if (HttpsServerManager::isRunning()) {
        if (mdnsStarted) {
            Serial.println("[WEB] Secure dashboard: https://prayer-bot.local/");
        } else {
            Serial.print("[WEB] Secure dashboard: https://");
            Serial.print((WiFi.getMode() & WIFI_AP) ? WiFi.softAPIP() : WiFi.localIP());
            Serial.println("/");
        }
    }
}

void stop() {
    HttpsServerManager::stop();
    if (server) {
        server->end();
        serverRunning = false;
    }
}

bool isRunning() {
    return serverRunning;
}

} // namespace WebServerManager
