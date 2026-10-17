#include "storage_manager.h"
#include "config.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// ============================================================
//  STORAGE MANAGER IMPLEMENTATION
// ============================================================

namespace StorageManager {
namespace {
    constexpr size_t MAX_STORAGE_FILE_BYTES = 64 * 1024;
    constexpr size_t MAX_STORAGE_PATH_BYTES = 80;
    SemaphoreHandle_t storageMutex = nullptr;
    bool storageReady = false;

    bool lockStorage() {
        return storageReady && storageMutex &&
               xSemaphoreTake(storageMutex, pdMS_TO_TICKS(1000)) == pdTRUE;
    }
}

bool init() {
    if (!storageMutex) storageMutex = xSemaphoreCreateMutex();
    if (!storageMutex) {
        Serial.println("[STORAGE] Could not create filesystem lock.");
        return false;
    }

    Preferences metadata;
    bool metadataAvailable = metadata.begin("prayerbot", false);
    const bool filesystemWasMounted = metadataAvailable &&
                                      metadata.getBool("littlefsReady", false);
    if (metadataAvailable) metadata.end();

    if (!LittleFS.begin(false)) {
        Serial.println("[STORAGE] LittleFS mount failed.");
        if (filesystemWasMounted) {
            Serial.println("[STORAGE] Preserving existing data; refusing automatic reformat.");
            return false;
        }
        Serial.println("[STORAGE] No prior filesystem marker; formatting first-use storage.");
        if (!LittleFS.begin(true)) {
            Serial.println("[STORAGE] LittleFS initialization failed.");
            return false;
        }
    }
    storageReady = true;
    if (!metadataAvailable) metadataAvailable = metadata.begin("prayerbot", false);
    if (metadataAvailable) {
        if (metadata.putBool("littlefsReady", true) == 0) {
            Serial.println("[STORAGE] Could not persist filesystem initialization marker.");
        }
        metadata.end();
    } else {
        Serial.println("[STORAGE] NVS metadata unavailable; filesystem data is mounted.");
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

    strlcpy(settings.owmApiKey, DEFAULT_OWM_API_KEY, sizeof(settings.owmApiKey));
    strlcpy(settings.geminiApiKey, DEFAULT_GEMINI_API_KEY, sizeof(settings.geminiApiKey));
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
    if (err || !doc.is<JsonObject>()) {
        Serial.print("[STORAGE] Settings parse error: ");
        Serial.println(err ? err.c_str() : "root is not an object");
        return false;
    }

    Settings loaded;
    loadDefaults(loaded);
    auto validString = [&doc](const char* key, size_t capacity) {
        if (doc[key].isUnbound()) return true;
        if (!doc[key].is<const char*>()) return false;
        const char* value = doc[key].as<const char*>();
        return strlen(value) < capacity;
    };
    auto number = [&doc](const char* key, float minimum, float maximum,
                         float fallback, float& result) {
        if (doc[key].isUnbound()) {
            result = fallback;
            return true;
        }
        if (!doc[key].is<int>() && !doc[key].is<float>()) return false;
        result = doc[key].as<float>();
        return isfinite(result) && result >= minimum && result <= maximum;
    };
    auto integer = [&doc](const char* key, int minimum, int maximum,
                          int fallback, int& result) {
        if (doc[key].isUnbound()) {
            result = fallback;
            return true;
        }
        if (!doc[key].is<int>()) return false;
        result = doc[key].as<int>();
        return result >= minimum && result <= maximum;
    };

    int intValue;
    bool valid = validString("city", sizeof(loaded.cityName)) &&
                 validString("tz", sizeof(loaded.timezone)) &&
                 validString("owmKey", sizeof(loaded.owmApiKey)) &&
                 validString("geminiKey", sizeof(loaded.geminiApiKey)) &&
                 number("lat", -90.0f, 90.0f, DEFAULT_LATITUDE, loaded.latitude) &&
                 number("lon", -180.0f, 180.0f, DEFAULT_LONGITUDE, loaded.longitude) &&
                 number("utcOff", -12.0f, 14.0f, DEFAULT_UTC_OFFSET, loaded.utcOffset);

    valid = valid &&
        integer("calcMethod", 0, 99, DEFAULT_CALC_METHOD, intValue) &&
        (intValue <= 5 || (intValue >= 7 && intValue <= 23) || intValue == 99);
    if (valid) loaded.calcMethod = static_cast<uint8_t>(intValue);
    valid = valid && integer("asrSchool", 0, 1, DEFAULT_ASR_SCHOOL, intValue);
    if (valid) loaded.asrSchool = static_cast<uint8_t>(intValue);
    valid = valid && integer("hijriOffset", -2, 2, DEFAULT_HIJRI_OFFSET, intValue);
    if (valid) loaded.hijriOffset = static_cast<int8_t>(intValue);
    if (doc["azanOn"].isUnbound()) loaded.azanEnabled = true;
    else if (doc["azanOn"].is<bool>()) loaded.azanEnabled = doc["azanOn"].as<bool>();
    else valid = false;
    valid = valid && integer("volume", 0, 100, DEFAULT_VOLUME, intValue);
    if (valid) loaded.volume = static_cast<uint8_t>(intValue);
    valid = valid && integer("focusMin", 1, 120, DEFAULT_FOCUS_MIN, intValue);
    if (valid) loaded.focusMinutes = static_cast<uint16_t>(intValue);
    valid = valid && integer("shortBrk", 2, 5, DEFAULT_SHORT_BREAK, intValue);
    if (valid) loaded.shortBreakMinutes = static_cast<uint16_t>(intValue);
    valid = valid && integer("longBrk", 1, 60, DEFAULT_LONG_BREAK, intValue);
    if (valid) loaded.longBreakMinutes = static_cast<uint16_t>(intValue);
    valid = valid && integer("pomCycles", 1, 10, DEFAULT_CYCLES, intValue);
    if (valid) loaded.pomodoroCycles = static_cast<uint8_t>(intValue);
    valid = valid && integer("sunriseOff", 0, 60, DEFAULT_SUNRISE_OFFSET, intValue);
    if (valid) loaded.sunriseOffset = static_cast<uint8_t>(intValue);
    valid = valid && integer("zawalOff", 0, 60, DEFAULT_ZAWAL_OFFSET, intValue);
    if (valid) loaded.zawalOffset = static_cast<uint8_t>(intValue);
    valid = valid && integer("sunsetOff", 0, 60, DEFAULT_SUNSET_OFFSET, intValue);
    if (valid) loaded.sunsetOffset = static_cast<uint8_t>(intValue);

    if (!valid) {
        Serial.println("[STORAGE] Settings contain invalid values; keeping safe defaults.");
        return false;
    }

    if (doc["city"].is<const char*>())
        strlcpy(loaded.cityName, doc["city"], sizeof(loaded.cityName));
    if (doc["tz"].is<const char*>())
        strlcpy(loaded.timezone, doc["tz"], sizeof(loaded.timezone));
    if (doc["owmKey"].is<const char*>())
        strlcpy(loaded.owmApiKey, doc["owmKey"], sizeof(loaded.owmApiKey));
    if (loaded.owmApiKey[0] == '\0') {
        strlcpy(loaded.owmApiKey, DEFAULT_OWM_API_KEY, sizeof(loaded.owmApiKey));
    }
    if (doc["geminiKey"].is<const char*>())
        strlcpy(loaded.geminiApiKey, doc["geminiKey"], sizeof(loaded.geminiApiKey));
    if (loaded.geminiApiKey[0] == '\0') {
        strlcpy(loaded.geminiApiKey, DEFAULT_GEMINI_API_KEY, sizeof(loaded.geminiApiKey));
    }
    if (loaded.cityName[0] == '\0' || loaded.timezone[0] == '\0') {
        Serial.println("[STORAGE] City and timezone cannot be empty; keeping safe defaults.");
        return false;
    }
    settings = loaded;

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
    doc["geminiKey"]   = settings.geminiApiKey;

    String json;
    if (serializeJson(doc, json) == 0 || json.isEmpty()) {
        Serial.println("[STORAGE] Could not serialize settings.");
        return false;
    }
    return writeFile(SETTINGS_PATH, json);
}

String readFile(const char* path) {
    if (!path || !lockStorage()) {
        Serial.println("[STORAGE] Read failed: storage unavailable or busy.");
        return "";
    }
    File file = LittleFS.open(path, "r");
    if (!file) {
        Serial.print("[STORAGE] Failed to open: ");
        Serial.println(path);
        xSemaphoreGive(storageMutex);
        return "";
    }
    const size_t fileSize = file.size();
    if (fileSize > MAX_STORAGE_FILE_BYTES) {
        Serial.printf("[STORAGE] Refusing oversized file (%u bytes): %s\n",
                      static_cast<unsigned>(fileSize), path);
        file.close();
        xSemaphoreGive(storageMutex);
        return "";
    }
    String content;
    if (!content.reserve(fileSize)) {
        Serial.printf("[STORAGE] Not enough memory to read: %s\n", path);
        file.close();
        xSemaphoreGive(storageMutex);
        return "";
    }
    char buffer[256];
    size_t remaining = fileSize;
    while (remaining > 0) {
        const size_t requested = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        const size_t count = file.readBytes(buffer, requested);
        if (count == 0 || !content.concat(buffer, count)) {
            Serial.printf("[STORAGE] Incomplete read: %s\n", path);
            content = "";
            break;
        }
        remaining -= count;
    }
    file.close();
    xSemaphoreGive(storageMutex);
    return content;
}

bool writeFile(const char* path, const String& content) {
    if (!path || strlen(path) > MAX_STORAGE_PATH_BYTES ||
        content.length() > MAX_STORAGE_FILE_BYTES || !lockStorage()) {
        Serial.println("[STORAGE] Write failed: invalid path, oversized file, storage unavailable, or busy.");
        return false;
    }
    String temporaryPath(path);
    temporaryPath += ".tmp";
    File file = LittleFS.open(temporaryPath.c_str(), "w");
    if (!file) {
        Serial.print("[STORAGE] Failed to write: ");
        Serial.println(path);
        xSemaphoreGive(storageMutex);
        return false;
    }
    size_t written = file.print(content);
    file.flush();
    file.close();
    if (written != content.length()) {
        Serial.print("[STORAGE] Incomplete write: ");
        Serial.println(path);
        LittleFS.remove(temporaryPath.c_str());
        xSemaphoreGive(storageMutex);
        return false;
    }

    File verification = LittleFS.open(temporaryPath.c_str(), "r");
    bool verified = verification && verification.size() == content.length();
    char buffer[256];
    size_t offset = 0;
    while (verified && offset < content.length()) {
        const size_t remaining = content.length() - offset;
        const size_t requested = remaining < sizeof(buffer) ? remaining : sizeof(buffer);
        const size_t count = verification.readBytes(buffer, requested);
        verified = count == requested &&
                   memcmp(buffer, content.c_str() + offset, count) == 0;
        offset += count;
    }
    if (verification) {
        verification.close();
    }
    if (!verified) {
        Serial.print("[STORAGE] Verification failed: ");
        Serial.println(path);
        LittleFS.remove(temporaryPath.c_str());
        xSemaphoreGive(storageMutex);
        return false;
    }
    bool renamed = LittleFS.rename(temporaryPath.c_str(), path);
    if (!renamed) {
        LittleFS.remove(path);
        renamed = LittleFS.rename(temporaryPath.c_str(), path);
    }
    if (!renamed) {
        Serial.print("[STORAGE] Could not replace: ");
        Serial.println(path);
        LittleFS.remove(temporaryPath.c_str());
        xSemaphoreGive(storageMutex);
        return false;
    }
    Serial.print("[STORAGE] Written: ");
    Serial.println(path);
    xSemaphoreGive(storageMutex);
    return true;
}

bool exists(const char* path) {
    if (!path || !lockStorage()) return false;
    const bool result = LittleFS.exists(path);
    xSemaphoreGive(storageMutex);
    return result;
}

bool removeFile(const char* path) {
    if (!path || !lockStorage()) {
        Serial.println("[STORAGE] Remove failed: storage unavailable or busy.");
        return false;
    }
    const bool result = !LittleFS.exists(path) || LittleFS.remove(path);
    if (!result) {
        Serial.print("[STORAGE] Failed to remove: ");
        Serial.println(path);
    }
    xSemaphoreGive(storageMutex);
    return result;
}

} // namespace StorageManager
