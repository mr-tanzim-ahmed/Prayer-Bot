#include "managers/prayer/calendar_manager.h"
#include <prayer_bot_config.h>
#include "managers/storage/storage_manager.h"
#include "managers/storage/cache_manager.h"
#include <ArduinoJson.h>
#include <time.h>

// ============================================================
//  CALENDAR MANAGER IMPLEMENTATION
//  Manages Hijri dates and Islamic event lookup.
// ============================================================

namespace {
    int8_t hijriOffset = 0;

    void normalizeHijriDate(HijriDate& date) {
        while (date.day < 1) {
            date.day += 30;
            if (date.month == 1) {
                date.month = 12;
                date.year--;
            } else {
                date.month--;
            }
        }

        while (date.day > 30) {
            date.day -= 30;
            if (date.month == 12) {
                date.month = 1;
                date.year++;
            } else {
                date.month++;
            }
        }
    }

    // Built-in Islamic events table
    const IslamicEvent EVENTS[] = {
        { 1,  1,  "Islamic New Year",  false},
        { 1,  10, "Ashura",            false},
        { 3,  12, "Eid-e-Miladunnabi", false},
        { 7,  27, "Shab-e-Meraj",      false},
        { 8,  15, "Shab-e-Barat",      false},
        { 9,  1,  "Ramadan Begins",    false},
        { 9,  27, "Shab-e-Qadr",       false},
        {10,  1,  "Eid ul-Fitr",       false},
        {12,  9,  "Day of Arafah",     false},
        {12,  10, "Eid ul-Adha",       false},
    };
    const int EVENT_COUNT = sizeof(EVENTS) / sizeof(EVENTS[0]);

    // Hijri month names
    const char* HIJRI_MONTHS[] = {
        "Muharram", "Safar", "Rabi al-Awwal", "Rabi al-Thani",
        "Jumada al-Ula", "Jumada al-Thani", "Rajab", "Shaban",
        "Ramadan", "Shawwal", "Dhul Qadah", "Dhul Hijjah"
    };
}

namespace CalendarManager {

void init(const Settings& settings) {
    hijriOffset = settings.hijriOffset;
    Serial.printf("[CALENDAR] Initialized with Hijri offset: %d\n", hijriOffset);
}

void getHijriDate(HijriDate& date) {
    // Read from cached prayer data which includes Hijri info per day
    String json = StorageManager::readFile(PRAYER_CACHE_PATH);
    const uint32_t cacheAge = CacheManager::getCacheAge(PRAYER_CACHE_PATH);
    const bool cacheFresh = cacheAge == 0xFFFFFFFF ||
        cacheAge <= CACHE_PRAYER_MAX_DAYS * 86400UL;
    if (!json.isEmpty() && cacheFresh) {
        JsonDocument doc;
        if (!deserializeJson(doc, json)) {
            if ((doc["version"] | 0) == 2) {
                JsonArray dailyBackups = doc["days"].as<JsonArray>();
                JsonObjectConst selectedDay;
                struct tm today = {};
                if (getLocalTime(&today, 0) &&
                    today.tm_year >= (2020 - 1900)) {
                    char todayDate[11];
                    snprintf(todayDate, sizeof(todayDate), "%04d-%02d-%02d",
                             today.tm_year + 1900, today.tm_mon + 1, today.tm_mday);
                    for (JsonObjectConst day : dailyBackups) {
                        if (strcmp(day["date"] | "", todayDate) == 0) {
                            selectedDay = day;
                            break;
                        }
                    }
                } else if (!dailyBackups.isNull() && dailyBackups.size() > 0) {
                    selectedDay = dailyBackups[0];
                }

                JsonObjectConst cachedHijri = selectedDay["hijri"];
                if (!cachedHijri.isNull()) {
                    date.day = cachedHijri["day"] | 1;
                    date.month = cachedHijri["month"] | 1;
                    date.year = cachedHijri["year"] | 1448;
                    date.monthName = String((const char*)(cachedHijri["monthName"] | "Muharram"));
                    date.designation = "AH";
                    date.day += hijriOffset;
                    normalizeHijriDate(date);
                    return;
                }
            }

            // Check single day cache format first
            JsonObject singleHijri = doc["hijri"];
            if (!singleHijri.isNull()) {
                date.day       = singleHijri["day"] | 1;
                date.month     = singleHijri["month"] | 1;
                date.year      = singleHijri["year"] | 1448;
                date.monthName = String((const char*)(singleHijri["monthName"] | "Muharram"));
                date.designation = "AH";
                date.day += hijriOffset;
                normalizeHijriDate(date);
                return;
            }

            // Check monthly calendar format
            struct tm t;
            if (getLocalTime(&t) && t.tm_year >= (2020 - 1900)) {
                int dayIndex = t.tm_mday - 1; // 0-indexed day of month
                JsonArray days = doc["days"].as<JsonArray>();
                
                if (dayIndex >= 0 && dayIndex < days.size()) {
                    JsonObject todayHijri = days[dayIndex]["hijri"];
                    if (!todayHijri.isNull()) {
                        date.day       = todayHijri["day"] | 1;
                        date.month     = todayHijri["month"] | 1;
                        date.year      = todayHijri["year"] | 1448;
                        date.monthName = String((const char*)(todayHijri["monthName"] | "Muharram"));
                        date.designation = "AH";

                        // Apply offset and keep the Hijri month/day valid after rollover.
                        date.day += hijriOffset;
                        normalizeHijriDate(date);
                        return;
                    }
                }
            }
        }
    }

    // Fallback: approximate Hijri date
    date.day = 1;
    date.month = 1;
    date.year = 1446;
    date.monthName = "Muharram";
    date.designation = "AH";
}

void getTodayEvent(const HijriDate& date, IslamicEvent& event) {
    event = {0, 0, "", false};

    // Check for Jumuah (Friday)
    struct tm t;
    if (getLocalTime(&t) && t.tm_year >= (2020 - 1900) && t.tm_wday == 5) {
        event = {0, 0, "Jumuah", true};
    }

    // Check fixed Hijri events (overrides Jumuah if both match)
    for (int i = 0; i < EVENT_COUNT; i++) {
        if (EVENTS[i].month == date.month && EVENTS[i].day == date.day) {
            event = EVENTS[i];
            return;
        }
    }
}

bool isEventComingSoon(const HijriDate& date, IslamicEvent& event, uint8_t withinDays) {
    for (int i = 0; i < EVENT_COUNT; i++) {
        if (EVENTS[i].month == date.month) {
            int diff = EVENTS[i].day - date.day;
            if (diff > 0 && diff <= withinDays) {
                event = EVENTS[i];
                return true;
            }
        }
    }
    return false;
}

void setHijriOffset(int8_t offset) {
    hijriOffset = offset;
    Serial.printf("[CALENDAR] Hijri offset set to: %d\n", offset);
}

} // namespace CalendarManager
