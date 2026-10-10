#include "pomodoro_stats.h"
#include "storage_manager.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <time.h>

namespace {
    constexpr char STATS_PATH[] = "/pomodoro_stats.json";
    constexpr size_t MAX_STORED_DAYS = 35;

    struct DayRecord {
        char date[11];
        uint16_t sessionsStarted;
        uint16_t sessionsCompleted;
        uint32_t focusSeconds;
    };

    DayRecord records[MAX_STORED_DAYS];
    size_t recordCount = 0;
    SemaphoreHandle_t statsMutex = nullptr;
    bool initialized = false;

    bool dateString(int daysAgo, char (&date)[11], char* weekday = nullptr) {
        time_t now;
        time(&now);
        struct tm local;
        if (!localtime_r(&now, &local) || local.tm_year < (2020 - 1900)) {
            return false;
        }

        local.tm_hour = 12;
        local.tm_min = 0;
        local.tm_sec = 0;
        local.tm_mday -= daysAgo;
        if (mktime(&local) == (time_t)-1) return false;

        if (strftime(date, sizeof(date), "%Y-%m-%d", &local) == 0) return false;
        if (weekday) strftime(weekday, 4, "%a", &local);
        return true;
    }

    int findRecord(const char* date) {
        for (size_t i = 0; i < recordCount; ++i) {
            if (strcmp(records[i].date, date) == 0) return (int)i;
        }
        return -1;
    }

    DayRecord* getTodayRecord() {
        char today[11];
        if (!dateString(0, today)) {
            Serial.println("[POMODORO-STATS] Local time unavailable; statistics were not recorded.");
            return nullptr;
        }

        int index = findRecord(today);
        if (index >= 0) return &records[index];
        if (recordCount >= MAX_STORED_DAYS) {
            memmove(records, records + 1, (MAX_STORED_DAYS - 1) * sizeof(DayRecord));
            --recordCount;
        }

        DayRecord& record = records[recordCount++];
        strlcpy(record.date, today, sizeof(record.date));
        record.sessionsStarted = 0;
        record.sessionsCompleted = 0;
        record.focusSeconds = 0;
        return &record;
    }

    bool saveRecords() {
        JsonDocument doc;
        JsonArray days = doc["days"].to<JsonArray>();
        for (size_t i = 0; i < recordCount; ++i) {
            JsonObject day = days.add<JsonObject>();
            day["date"] = records[i].date;
            day["started"] = records[i].sessionsStarted;
            day["completed"] = records[i].sessionsCompleted;
            day["focusSeconds"] = records[i].focusSeconds;
        }

        String json;
        serializeJson(doc, json);
        if (StorageManager::writeFile(STATS_PATH, json)) return true;
        Serial.println("[POMODORO-STATS] Failed to persist weekly history.");
        return false;
    }
}

namespace PomodoroStats {

bool init() {
    if (!statsMutex) statsMutex = xSemaphoreCreateMutex();
    if (!statsMutex) {
        Serial.println("[POMODORO-STATS] Could not create statistics lock.");
        return false;
    }

    if (StorageManager::exists(STATS_PATH)) {
        String json = StorageManager::readFile(STATS_PATH);
        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, json);
        if (error) {
            Serial.printf("[POMODORO-STATS] History parse error: %s\n", error.c_str());
            return false;
        }

        JsonArrayConst days = doc["days"].as<JsonArrayConst>();
        for (JsonObjectConst day : days) {
            if (recordCount >= MAX_STORED_DAYS) break;
            const char* date = day["date"];
            if (!date || strlen(date) != 10) continue;
            DayRecord& record = records[recordCount++];
            strlcpy(record.date, date, sizeof(record.date));
            record.sessionsStarted = day["started"] | 0;
            record.sessionsCompleted = day["completed"] | 0;
            record.focusSeconds = day["focusSeconds"] | 0;
        }
    }

    initialized = true;
    Serial.printf("[POMODORO-STATS] Loaded %u daily records.\n", (unsigned)recordCount);
    return true;
}

bool recordSessionStarted() {
    if (!initialized || xSemaphoreTake(statsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        Serial.println("[POMODORO-STATS] Could not lock statistics for session start.");
        return false;
    }
    DayRecord* today = getTodayRecord();
    bool saved = false;
    if (today) {
        ++today->sessionsStarted;
        saved = saveRecords();
    }
    xSemaphoreGive(statsMutex);
    return saved;
}

bool recordFocusCompleted(uint16_t focusMinutes) {
    if (!initialized || xSemaphoreTake(statsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        Serial.println("[POMODORO-STATS] Could not lock statistics for completed focus.");
        return false;
    }
    DayRecord* today = getTodayRecord();
    bool saved = false;
    if (today) {
        ++today->sessionsCompleted;
        today->focusSeconds += (uint32_t)focusMinutes * 60UL;
        saved = saveRecords();
    }
    xSemaphoreGive(statsMutex);
    return saved;
}

bool getWeeklyStats(JsonDocument& output) {
    if (!initialized || xSemaphoreTake(statsMutex, pdMS_TO_TICKS(1000)) != pdTRUE) {
        Serial.println("[POMODORO-STATS] Could not lock statistics for dashboard read.");
        return false;
    }

    JsonArray week = output["week"].to<JsonArray>();
    uint32_t totalStarted = 0;
    uint32_t totalCompleted = 0;
    uint32_t totalFocusSeconds = 0;

    for (int ago = 6; ago >= 0; --ago) {
        char date[11];
        char weekday[4];
        if (!dateString(ago, date, weekday)) {
            xSemaphoreGive(statsMutex);
            Serial.println("[POMODORO-STATS] Local time unavailable; weekly report omitted.");
            return false;
        }

        uint16_t started = 0;
        uint16_t completed = 0;
        uint32_t focusSeconds = 0;
        int index = findRecord(date);
        if (index >= 0) {
            started = records[index].sessionsStarted;
            completed = records[index].sessionsCompleted;
            focusSeconds = records[index].focusSeconds;
        }

        JsonObject day = week.add<JsonObject>();
        day["date"] = date;
        day["weekday"] = weekday;
        day["started"] = started;
        day["completed"] = completed;
        day["focusMinutes"] = focusSeconds / 60;
        totalStarted += started;
        totalCompleted += completed;
        totalFocusSeconds += focusSeconds;
    }

    output["totalStarted"] = totalStarted;
    output["totalCompleted"] = totalCompleted;
    output["totalFocusMinutes"] = totalFocusSeconds / 60;
    xSemaphoreGive(statsMutex);
    return true;
}

} // namespace PomodoroStats
