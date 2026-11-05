#include "managers/prayer/calendar_manager.h"
#include <prayer_bot_config.h>
#include "managers/storage/storage_manager.h"
#include "managers/storage/cache_manager.h"
#include <ArduinoJson.h>
#include <time.h>
#include <freertos/FreeRTOS.h>

// ============================================================
//  CALENDAR MANAGER IMPLEMENTATION
//  Manages Hijri dates and Islamic event lookup.
// ============================================================

namespace {
    int8_t hijriOffset = 0;
    portMUX_TYPE calendarStateMux = portMUX_INITIALIZER_UNLOCKED;

    // Hijri month names
    const char* HIJRI_MONTHS[] = {
        "Muharram", "Safar", "Rabi al-Awwal", "Rabi al-Thani",
        "Jumada al-Ula", "Jumada al-Thani", "Rajab", "Shaban",
        "Ramadan", "Shawwal", "Dhul Qadah", "Dhul Hijjah"
    };

    void normalizeHijriDate(HijriDate& date, int day, int8_t offset) {
        const uint8_t originalMonth = date.month;
        while (day < 1) {
            day += 30;
            if (date.month == 1) {
                date.month = 12;
                date.year--;
            } else {
                date.month--;
            }
        }

        while (day > 30) {
            day -= 30;
            if (date.month == 12) {
                date.month = 1;
                date.year++;
            } else {
                date.month++;
            }
        }

        if (date.month != originalMonth && date.month >= 1 && date.month <= 12) {
            date.monthName = HIJRI_MONTHS[date.month - 1];
        }
        date.day = day;
    }

    bool loadCachedHijriDate(JsonObjectConst cachedHijri, HijriDate& date,
                             int8_t offset) {
        JsonVariantConst cachedDay = cachedHijri["day"];
        JsonVariantConst cachedMonth = cachedHijri["month"];
        JsonVariantConst cachedYear = cachedHijri["year"];
        if (cachedHijri.isNull() || !cachedDay.is<int>() ||
            !cachedMonth.is<int>() || !cachedYear.is<int>()) {
            return false;
        }

        const int day = cachedDay.as<int>();
        const int month = cachedMonth.as<int>();
        const int year = cachedYear.as<int>();
        if (day < 1 || day > 30 || month < 1 || month > 12 ||
            year < 1 || year > UINT16_MAX) {
            return false;
        }

        date.day = day;
        date.month = month;
        date.year = year;
        const char* monthName = cachedHijri["monthName"] | "";
        date.monthName = monthName[0] ? monthName : HIJRI_MONTHS[month - 1];
        date.designation = "AH";
        normalizeHijriDate(date, day + offset, offset);
        return true;
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

}

namespace CalendarManager {

void init(const Settings& settings) {
    portENTER_CRITICAL(&calendarStateMux);
    hijriOffset = settings.hijriOffset;
    portEXIT_CRITICAL(&calendarStateMux);
    Serial.printf("[CALENDAR] Initialized with Hijri offset: %d\n",
                  settings.hijriOffset);
}

void getHijriDate(HijriDate& date) {
    portENTER_CRITICAL(&calendarStateMux);
    const int8_t offset = hijriOffset;
    portEXIT_CRITICAL(&calendarStateMux);

    // Read from cached prayer data which includes Hijri info per day
    String json = StorageManager::readFile(PRAYER_CACHE_PATH);
    const uint32_t cacheAge = CacheManager::getCacheAge(PRAYER_CACHE_PATH);
    const bool cacheFresh = cacheAge != 0xFFFFFFFF &&
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
                if (loadCachedHijriDate(cachedHijri, date, offset)) {
                    return;
                }
            }

            // Check single day cache format first
            JsonObjectConst singleHijri = doc["hijri"];
            if (loadCachedHijriDate(singleHijri, date, offset)) {
                return;
            }

            // Check monthly calendar format
            struct tm t;
            if (getLocalTime(&t) && t.tm_year >= (2020 - 1900)) {
                int dayIndex = t.tm_mday - 1; // 0-indexed day of month
                JsonArray days = doc["days"].as<JsonArray>();
                
                if (dayIndex >= 0 && dayIndex < days.size()) {
                    JsonObjectConst todayHijri = days[dayIndex]["hijri"];
                    if (loadCachedHijriDate(todayHijri, date, offset)) {
                        return;
                    }
                }
            }
        }
    }

    // Zero year represents an unavailable date; never substitute a stale fixed date.
    date.day = 0;
    date.month = 0;
    date.year = 0;
    date.monthName = "";
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
    event = {0, 0, "", false};
    if (date.month < 1 || date.month > 12 || date.day < 1 || date.day > 30)
        return false;

    int soonestDays = 361;
    for (int i = 0; i < EVENT_COUNT; i++) {
        const int monthsAhead = (EVENTS[i].month - date.month + 12) % 12;
        int diff = monthsAhead * 30 + EVENTS[i].day - date.day;
        if (diff <= 0) diff += 12 * 30;
        if (diff > 0 && diff <= withinDays && diff < soonestDays) {
            event = EVENTS[i];
            soonestDays = diff;
        }
    }
    return soonestDays <= withinDays;
}

void setHijriOffset(int8_t offset) {
    portENTER_CRITICAL(&calendarStateMux);
    hijriOffset = offset;
    portEXIT_CRITICAL(&calendarStateMux);
    Serial.printf("[CALENDAR] Hijri offset set to: %d\n", offset);
}

} // namespace CalendarManager
