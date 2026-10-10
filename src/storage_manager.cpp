#include "storage_manager.h"
#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>

// ============================================================
//  STORAGE MANAGER IMPLEMENTATION
// ============================================================

namespace StorageManager {

bool init() {
    if (!LittleFS.begin(true)) {
        Serial.println("[STORAGE] LittleFS mount failed!");
        return false;
    }
    Serial.println("[STORAGE] LittleFS mounted.");
    return true;
}

void loadDefaults(Settings& settings) {
    strlcpy(settings.cityName, DEFAULT_CITY_NAME, sizeof(settings.cityName));
    settings.latitude         = DEFAULT_LATITUDE;
    settings.longitude        = DEFAULT_LONGITUDE;
    strlcpy(settings.timezone, DEFAULT_TIMEZONE, sizeof(settings.timezone));
    settings.utcOffset        = DEFAULT_UTC_OFFSET;

    settings.calcMethod       = DEFAULT_CALC_METHOD;
    settings.asrSchool        = DEFAULT_ASR_SCHOOL;
    settings.hijriOffset      = DEFAULT_HIJRI_OFFSET;

    settings.azanEnabled      = true;
    settings.volume           = DEFAULT_VOLUME;

    settings.focusMinutes     = DEFAULT_FOCUS_MIN;
    settings.shortBreakMinutes = DEFAULT_SHORT_BREAK;
    settings.longBreakMinutes = DEFAULT_LONG_BREAK;
    settings.pomodoroCycles   = DEFAULT_CYCLES;

    settings.sunriseOffset    = DEFAULT_SUNRISE_OFFSET;
    settings.zawalOffset      = DEFAULT_ZAWAL_OFFSET;
    settings.sunsetOffset     = DEFAULT_SUNSET_OFFSET;

    settings.owmApiKey[0]     = '\0';
}

bool loadSettings(Settings& settings) {
    loadDefaults(settings);

    if (!exists(SETTINGS_PATH)) {
        Serial.println("[STORAGE] No settings file. Using defaults.");
        return saveSettings(settings);
    }

    String json = readFile(SETTINGS_PATH);
    if (json.isEmpty()) {
        Serial.println("[STORAGE] Settings file empty. Using defaults.");
        return false;
    }

    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, json);
    if (err) {
        Serial.print("[STORAGE] Settings parse error: ");
        Serial.println(err.c_str());
        return false;
    }

    // Location
    if (doc["city"].is<const char*>())
        strlcpy(settings.cityName, doc["city"], sizeof(settings.cityName));
    settings.latitude    = doc["lat"]    | DEFAULT_LATITUDE;
    settings.longitude   = doc["lon"]    | DEFAULT_LONGITUDE;
    if (doc["tz"].is<const char*>())
        strlcpy(settings.timezone, doc["tz"], sizeof(settings.timezone));
    settings.utcOffset   = doc["utcOff"] | DEFAULT_UTC_OFFSET;

    // Prayer
    settings.calcMethod  = doc["calcMethod"]  | DEFAULT_CALC_METHOD;
    settings.asrSchool   = doc["asrSchool"]   | DEFAULT_ASR_SCHOOL;
    settings.hijriOffset = doc["hijriOffset"] | DEFAULT_HIJRI_OFFSET;

    // Azan
    settings.azanEnabled = doc["azanOn"]  | true;
    settings.volume      = doc["volume"]  | DEFAULT_VOLUME;

    // Pomodoro
    settings.focusMinutes      = doc["focusMin"]  | DEFAULT_FOCUS_MIN;
    settings.shortBreakMinutes = doc["shortBrk"]  | DEFAULT_SHORT_BREAK;
    if (settings.shortBreakMinutes < 2 || settings.shortBreakMinutes > 5) {
        settings.shortBreakMinutes = DEFAULT_SHORT_BREAK;
    }
    settings.longBreakMinutes  = doc["longBrk"]   | DEFAULT_LONG_BREAK;
    settings.pomodoroCycles    = doc["pomCycles"]  | DEFAULT_CYCLES;

    // Prohibited offsets
    settings.sunriseOffset = doc["sunriseOff"] | DEFAULT_SUNRISE_OFFSET;
    settings.zawalOffset   = doc["zawalOff"]   | DEFAULT_ZAWAL_OFFSET;
    settings.sunsetOffset  = doc["sunsetOff"]  | DEFAULT_SUNSET_OFFSET;

    // API key
    if (doc["owmKey"].is<const char*>())
        strlcpy(settings.owmApiKey, doc["owmKey"], sizeof(settings.owmApiKey));

    Serial.println("[STORAGE] Settings loaded from flash.");
    return true;
}

bool saveSettings(const Settings& settings) {
    JsonDocument doc;

    doc["city"]       = settings.cityName;
    doc["lat"]        = settings.latitude;
    doc["lon"]        = settings.longitude;
    doc["tz"]         = settings.timezone;
    doc["utcOff"]     = settings.utcOffset;

    doc["calcMethod"]  = settings.calcMethod;
    doc["asrSchool"]   = settings.asrSchool;
    doc["hijriOffset"] = settings.hijriOffset;

    doc["azanOn"]      = settings.azanEnabled;
    doc["volume"]      = settings.volume;

    doc["focusMin"]    = settings.focusMinutes;
    doc["shortBrk"]    = settings.shortBreakMinutes;
    doc["longBrk"]     = settings.longBreakMinutes;
    doc["pomCycles"]   = settings.pomodoroCycles;

    doc["sunriseOff"]  = settings.sunriseOffset;
    doc["zawalOff"]    = settings.zawalOffset;
    doc["sunsetOff"]   = settings.sunsetOffset;

    doc["owmKey"]      = settings.owmApiKey;

    String json;
    serializeJson(doc, json);
    return writeFile(SETTINGS_PATH, json);
}

String readFile(const char* path) {
    File file = LittleFS.open(path, "r");
    if (!file) {
        Serial.print("[STORAGE] Failed to open: ");
        Serial.println(path);
        return "";
    }
    String content = file.readString();
    file.close();
    return content;
}

bool writeFile(const char* path, const String& content) {
    File file = LittleFS.open(path, "w");
    if (!file) {
        Serial.print("[STORAGE] Failed to write: ");
        Serial.println(path);
        return false;
    }
    size_t written = file.print(content);
    file.close();
    if (written != content.length()) {
        Serial.print("[STORAGE] Incomplete write: ");
        Serial.println(path);
        return false;
    }
    Serial.print("[STORAGE] Written: ");
    Serial.println(path);
    return true;
}

bool exists(const char* path) {
    return LittleFS.exists(path);
}

} // namespace StorageManager
