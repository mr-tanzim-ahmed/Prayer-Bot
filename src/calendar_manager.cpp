#include "calendar_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <ArduinoJson.h>
#include <time.h>

// ============================================================
//  CALENDAR MANAGER IMPLEMENTATION
//  Manages Hijri dates and Islamic event lookup.
// ============================================================

namespace {
    int8_t hijriOffset = 0;

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
    if (!json.isEmpty()) {
        JsonDocument doc;
        if (!deserializeJson(doc, json)) {
            struct tm t;
            if (getLocalTime(&t)) {
                int dayIndex = t.tm_mday - 1; // 0-indexed day of month
                JsonArray days = doc["days"].as<JsonArray>();
                
                if (dayIndex >= 0 && dayIndex < days.size()) {
                    JsonObject todayHijri = days[dayIndex]["hijri"];
                    if (!todayHijri.isNull()) {
                        date.day       = todayHijri["day"] | 1;
                        date.month     = todayHijri["month"] | 1;
                        date.year      = todayHijri["year"] | 1446;
                        date.monthName = String((const char*)(todayHijri["monthName"] | "Muharram"));
                        date.designation = "AH";

                        // Apply offset
                        date.day += hijriOffset;
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
    if (getLocalTime(&t) && t.tm_wday == 5) {
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
