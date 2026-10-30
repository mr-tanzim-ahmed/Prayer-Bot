#include "cache_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <LittleFS.h>
#include <ArduinoJson.h>
#include <time.h>

namespace CacheManager {

namespace {
    bool isManagedCachePath(const char* path) {
        return path &&
            (strcmp(path, PRAYER_CACHE_PATH) == 0 ||
             strcmp(path, WEATHER_CACHE_PATH) == 0 ||
             strcmp(path, FESTIVAL_TABLE_PATH) == 0);
    }
}

static JsonDocument metaDoc;

void init() {
    constexpr size_t MAX_CACHE_META_BYTES = 2048;
    metaDoc.clear();
    if (!LittleFS.exists(CACHE_META_PATH)) {
        metaDoc.to<JsonObject>();
        return;
    }

    File file = LittleFS.open(CACHE_META_PATH, "r");
    if (!file) {
        Serial.println("[CACHE] Could not open metadata; starting with empty metadata.");
        metaDoc.to<JsonObject>();
        return;
    }
    if (file.size() > MAX_CACHE_META_BYTES) {
        Serial.println("[CACHE] Metadata exceeds size limit; starting with empty metadata.");
        file.close();
        metaDoc.to<JsonObject>();
        return;
    }

    DeserializationError error = deserializeJson(metaDoc, file);
    file.close();
    if (error || !metaDoc.is<JsonObject>()) {
        Serial.printf("[CACHE] Invalid metadata; starting with empty metadata: %s\n",
                      error ? error.c_str() : "root is not an object");
        metaDoc.clear();
        metaDoc.to<JsonObject>();
    }
}

static void saveMeta() {
    String json;
    serializeJson(metaDoc, json);
    if (!StorageManager::writeFile(CACHE_META_PATH, json)) {
        Serial.println("[CACHE] Failed to save cache metadata.");
    }
}

static uint32_t getCurrentTime() {
    time_t now;
    time(&now);
    return (uint32_t)now;
}

bool isCacheValid(const char* path, uint32_t maxAgeDays) {
    if (!isManagedCachePath(path) || !metaDoc[path].is<uint32_t>())
        return false;
    uint32_t timestamp = metaDoc[path] | 0;
    if (timestamp == 0) return false;
    
    uint32_t now = getCurrentTime();
    // If time is not synced (e.g., year is 1970), assume invalid
    if (now < 1000000000) return false; 
    
    uint32_t ageSecs = now > timestamp ? (now - timestamp) : 0;
    return ageSecs <= (maxAgeDays * 86400);
}

void markUpdated(const char* path) {
    if (!isManagedCachePath(path)) {
        Serial.println("[CACHE] Refusing to track an unknown cache path.");
        return;
    }
    uint32_t now = getCurrentTime();
    if (now > 1000000000) {
        metaDoc[path] = now;
        saveMeta();
    }
}

void cleanExpired(uint32_t maxAgeDays) {
    uint32_t now = getCurrentTime();
    if (now < 1000000000) return; // Time not synced
    constexpr size_t MAX_EXPIRED_ENTRIES = 16;
    struct ExpiredEntry {
        char path[64];
    };
    ExpiredEntry expired[MAX_EXPIRED_ENTRIES];
    size_t expiredCount = 0;
    JsonObject root = metaDoc.as<JsonObject>();
    
    for (JsonPair kv : root) {
        const char* path = kv.key().c_str();
        if (!isManagedCachePath(path)) continue;
        if (!kv.value().is<uint32_t>()) {
            Serial.printf("[CACHE] Ignoring invalid timestamp for: %s\n", path);
            continue;
        }
        
        uint32_t timestamp = kv.value().as<uint32_t>();
        if (timestamp == 0) continue;
        uint32_t ageSecs = now > timestamp ? (now - timestamp) : 0;
        
        // Use specific max days based on path, or default maxAgeDays
        uint32_t maxDays = maxAgeDays;
        if (strcmp(path, PRAYER_CACHE_PATH) == 0) maxDays = CACHE_PRAYER_MAX_DAYS;
        else if (strcmp(path, WEATHER_CACHE_PATH) == 0) maxDays = CACHE_WEATHER_MAX_DAYS;
        else if (strcmp(path, FESTIVAL_TABLE_PATH) == 0) maxDays = CACHE_FESTIVAL_MAX_DAYS;
        
        if (ageSecs > (maxDays * 86400UL)) {
            Serial.printf("[CACHE] Expired: %s (age %u days)\n", path, ageSecs / 86400);
            if (expiredCount == MAX_EXPIRED_ENTRIES) {
                Serial.println("[CACHE] Expired metadata limit reached; remaining entries deferred.");
                break;
            }
            strlcpy(expired[expiredCount++].path, path, sizeof(expired[0].path));
        }
    }

    bool changed = false;
    for (size_t i = 0; i < expiredCount; ++i) {
        const char* path = expired[i].path;
        if (StorageManager::removeFile(path)) {
            root.remove(path);
            changed = true;
        } else {
            Serial.printf("[CACHE] Could not remove expired file: %s\n", path);
        }
    }

    if (changed) saveMeta();
}

void cleanAll() {
    JsonObject root = metaDoc.as<JsonObject>();
    const char* const cachePaths[] = {
        PRAYER_CACHE_PATH,
        WEATHER_CACHE_PATH,
        FESTIVAL_TABLE_PATH
    };
    bool changed = false;
    bool complete = true;
    for (const char* path : cachePaths) {
        const bool existed = StorageManager::exists(path);
        if (!StorageManager::removeFile(path)) {
            Serial.printf("[CACHE] Could not remove cache file: %s\n", path);
            complete = false;
            continue;
        }
        if (existed || !root[path].isUnbound()) changed = true;
        root.remove(path);
    }
    if (changed) saveMeta();
    Serial.println(complete
        ? "[CACHE] All managed caches cleared."
        : "[CACHE] Cache cleanup incomplete; see errors above.");
}

uint32_t getCacheAge(const char* path) {
    if (!isManagedCachePath(path) || !metaDoc[path].is<uint32_t>())
        return 0xFFFFFFFF;
    uint32_t timestamp = metaDoc[path] | 0;
    uint32_t now = getCurrentTime();
    if (now < 1000000000 || timestamp == 0) return 0xFFFFFFFF;
    return now > timestamp ? (now - timestamp) : 0;
}

size_t getTotalCacheSize() {
    size_t total = 0;
    JsonObject root = metaDoc.as<JsonObject>();
    for (JsonPair kv : root) {
        const char* path = kv.key().c_str();
        if (!isManagedCachePath(path)) continue;
        if (LittleFS.exists(path)) {
            File f = LittleFS.open(path, "r");
            if (f) {
                total += f.size();
                f.close();
            }
        }
    }
    return total;
}

void listCacheFiles() {
    Serial.println("--- Cache Files ---");
    JsonObject root = metaDoc.as<JsonObject>();
    for (JsonPair kv : root) {
        const char* path = kv.key().c_str();
        if (!isManagedCachePath(path)) continue;
        uint32_t ageSecs = getCacheAge(path);
        size_t size = 0;
        if (LittleFS.exists(path)) {
            File f = LittleFS.open(path, "r");
            if (f) {
                size = f.size();
                f.close();
            }
        }
        Serial.printf("File: %s | Age: %u secs | Size: %u bytes\n", path, ageSecs, size);
    }
    Serial.println("-------------------");
}

} // namespace CacheManager
