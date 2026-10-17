#include "prayer_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include "cache_manager.h"

// ============================================================
//  PRAYER MANAGER IMPLEMENTATION
//  Fetches prayer times from Aladhan API, caches monthly
//  data, and computes next prayer / prohibited times.
// ============================================================

namespace {
    DailyPrayers cachedPrayers[31];  // Up to 31 days in a month
    int cachedMonth = -1;
    int cachedYear = -1;
    int cachedDaysCount = 0;
    PrayerName lastAzanPrayer = PRAYER_NONE;
    int lastAzanDay = -1;

    bool parseTime(const char* str, PrayerTime& time) {
        if (!str || strlen(str) < 5 ||
            str[0] < '0' || str[0] > '9' ||
            str[1] < '0' || str[1] > '9' ||
            str[2] != ':' ||
            str[3] < '0' || str[3] > '9' ||
            str[4] < '0' || str[4] > '9') {
            return false;
        }
        const uint8_t hour = (str[0] - '0') * 10 + (str[1] - '0');
        const uint8_t minute = (str[3] - '0') * 10 + (str[4] - '0');
        if (hour > 23 || minute > 59) return false;
        time = {hour, minute};
        return true;
    }

    int timeToMinutes(PrayerTime pt) {
        return pt.hour * 60 + pt.minute;
    }

    int currentTimeMinutes() {
        struct tm t;
        if (!getLocalTime(&t)) return -1;
        return t.tm_hour * 60 + t.tm_min;
    }

    int currentDay() {
        struct tm t;
        if (!getLocalTime(&t)) return -1;
        return t.tm_mday;
    }

    int currentMonth() {
        struct tm t;
        if (!getLocalTime(&t)) return -1;
        return t.tm_mon + 1;
    }

    int currentYear() {
        struct tm t;
        if (!getLocalTime(&t)) return -1;
        return t.tm_year + 1900;
    }

    int currentSeconds() {
        struct tm t;
        if (!getLocalTime(&t)) return -1;
        return t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
    }
}

namespace PrayerManager {

bool init(const Settings& settings) {
    // Try loading from cache first
    String json = StorageManager::readFile(PRAYER_CACHE_PATH);
    if (!json.isEmpty()) {
        JsonDocument doc;
        if (!deserializeJson(doc, json) && doc["days"].is<JsonArray>()) {
            const int cacheMonth = doc["month"] | -1;
            const int cacheYear = doc["year"] | -1;

            int m = currentMonth();
            int y = currentYear();

            if (cacheMonth == m && cacheYear == y) {
                DailyPrayers loadedPrayers[31];
                int loadedCount = 0;
                JsonArray days = doc["days"].as<JsonArray>();
                for (JsonObject day : days) {
                    if (loadedCount >= 31) break;
                    DailyPrayers& prayer = loadedPrayers[loadedCount];
                    if (!parseTime(day["fajr"], prayer.fajr) ||
                        !parseTime(day["sunrise"], prayer.sunrise) ||
                        !parseTime(day["dhuhr"], prayer.dhuhr) ||
                        !parseTime(day["asr"], prayer.asr) ||
                        !parseTime(day["maghrib"], prayer.maghrib) ||
                        !parseTime(day["isha"], prayer.isha)) {
                        loadedCount = 0;
                        break;
                    }
                    ++loadedCount;
                }
                if (loadedCount >= 28) {
                    memcpy(cachedPrayers, loadedPrayers,
                           loadedCount * sizeof(DailyPrayers));
                    cachedMonth = cacheMonth;
                    cachedYear = cacheYear;
                    cachedDaysCount = loadedCount;
                    Serial.printf("[PRAYER] Loaded %d days from cache.\n", cachedDaysCount);
                    return true;
                }
            }
        }
    }

    // Fetch from API
    return refresh(settings);
}

bool refresh(const Settings& settings) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[PRAYER] WiFi not connected. Cannot fetch.");
        return false;
    }

    int month = currentMonth();
    int year  = currentYear();
    if (month < 0 || year < 0) {
        Serial.println("[PRAYER] Time not synced.");
        return false;
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    String url = String(ALADHAN_API_BASE) + "/calendar"
        "?latitude=" + String(settings.latitude, 4) +
        "&longitude=" + String(settings.longitude, 4) +
        "&method=" + String(settings.calcMethod) +
        "&school=" + String(settings.asrSchool) +
        "&month=" + String(month) +
        "&year=" + String(year);

    Serial.println("[PRAYER] Fetching monthly prayer times...");

    if (!http.begin(client, url)) {
        Serial.println("[PRAYER] HTTP begin failed.");
        return false;
    }

    http.setTimeout(20000);
    int code = http.GET();

    if (code != HTTP_CODE_OK) {
        Serial.printf("[PRAYER] HTTP error: %d\n", code);
        http.end();
        return false;
    }

    // Set up filter to save memory
    JsonDocument filter;
    filter["data"][0]["timings"]["Fajr"] = true;
    filter["data"][0]["timings"]["Sunrise"] = true;
    filter["data"][0]["timings"]["Dhuhr"] = true;
    filter["data"][0]["timings"]["Asr"] = true;
    filter["data"][0]["timings"]["Maghrib"] = true;
    filter["data"][0]["timings"]["Isha"] = true;
    filter["data"][0]["date"]["hijri"]["day"] = true;
    filter["data"][0]["date"]["hijri"]["month"]["number"] = true;
    filter["data"][0]["date"]["hijri"]["month"]["en"] = true;
    filter["data"][0]["date"]["hijri"]["year"] = true;

    // Parse response from stream
    JsonDocument doc;
    DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
    http.end();

    if (err) {
        Serial.print("[PRAYER] JSON error: ");
        Serial.println(err.c_str());
        return false;
    }

    JsonArray data = doc["data"].as<JsonArray>();
    if (!doc["data"].is<JsonArray>() || data.size() < 28 || data.size() > 31) {
        Serial.println("[PRAYER] API response did not contain a complete monthly schedule.");
        return false;
    }

    // Build cache document
    JsonDocument cacheDoc;
    cacheDoc["month"] = month;
    cacheDoc["year"]  = year;
    JsonArray cacheDays = cacheDoc["days"].to<JsonArray>();
    DailyPrayers fetchedPrayers[31];
    int fetchedCount = 0;

    for (JsonObject dayObj : data) {
        if (fetchedCount >= 31) break;
        JsonObject timings = dayObj["timings"].as<JsonObject>();
        JsonObject hijri = dayObj["date"]["hijri"];
        const char* rawTimes[] = {
            timings["Fajr"], timings["Sunrise"], timings["Dhuhr"],
            timings["Asr"], timings["Maghrib"], timings["Isha"]
        };
        DailyPrayers& prayer = fetchedPrayers[fetchedCount];
        if (!parseTime(rawTimes[0], prayer.fajr) ||
            !parseTime(rawTimes[1], prayer.sunrise) ||
            !parseTime(rawTimes[2], prayer.dhuhr) ||
            !parseTime(rawTimes[3], prayer.asr) ||
            !parseTime(rawTimes[4], prayer.maghrib) ||
            !parseTime(rawTimes[5], prayer.isha)) {
            Serial.printf("[PRAYER] Invalid prayer time in API response at day %d.\n",
                          fetchedCount + 1);
            return false;
        }

        // Aladhan returns times like "05:23 (EET)" - extract just HH:MM
        String timeStrings[6];
        for (size_t i = 0; i < 6; ++i) {
            timeStrings[i] = String(rawTimes[i]).substring(0, 5);
        }

        JsonObject cacheDay = cacheDays.add<JsonObject>();
        cacheDay["fajr"]    = timeStrings[0];
        cacheDay["sunrise"] = timeStrings[1];
        cacheDay["dhuhr"]   = timeStrings[2];
        cacheDay["asr"]     = timeStrings[3];
        cacheDay["maghrib"] = timeStrings[4];
        cacheDay["isha"]    = timeStrings[5];

        // Save Hijri info to cache
        JsonObject cacheHijri = cacheDay["hijri"].to<JsonObject>();
        cacheHijri["day"] = hijri["day"] | 1;
        cacheHijri["month"] = hijri["month"]["number"] | 1;
        cacheHijri["monthName"] = hijri["month"]["en"] | "Muharram";
        cacheHijri["year"] = hijri["year"] | 1446;

        ++fetchedCount;
    }
    if (fetchedCount < 28) {
        Serial.println("[PRAYER] API response ended before the monthly schedule was complete.");
        return false;
    }

    memcpy(cachedPrayers, fetchedPrayers, fetchedCount * sizeof(DailyPrayers));
    cachedDaysCount = fetchedCount;
    cachedMonth = month;
    cachedYear  = year;

    // Save to flash
    String cacheJson;
    if (serializeJson(cacheDoc, cacheJson) > 0 &&
        StorageManager::writeFile(PRAYER_CACHE_PATH, cacheJson)) {
        CacheManager::markUpdated(PRAYER_CACHE_PATH);
    } else {
        Serial.println("[PRAYER] Schedule is active, but the cache could not be saved.");
    }

    Serial.printf("[PRAYER] Cached %d days for %d/%d.\n", cachedDaysCount, month, year);
    return true;
}

void getTodayPrayers(DailyPrayers& prayers) {
    int day = currentDay();
    if (day >= 1 && day <= cachedDaysCount) {
        prayers = cachedPrayers[day - 1];
    }
}

void updateNextPrayer(NextPrayerInfo& next, ProhibitedTimes& prohibited,
                      const Settings& settings) {
    DailyPrayers today;
    getTodayPrayers(today);

    int nowMin = currentTimeMinutes();
    int nowSec = currentSeconds();
    if (nowMin < 0 || nowSec < 0) return;

    // Prayer times in order
    struct { PrayerName name; PrayerTime time; } prayers[] = {
        {PRAYER_FAJR,    today.fajr},
        {PRAYER_SUNRISE, today.sunrise},
        {PRAYER_DHUHR,   today.dhuhr},
        {PRAYER_ASR,     today.asr},
        {PRAYER_MAGHRIB, today.maghrib},
        {PRAYER_ISHA,    today.isha}
    };

    // Find next prayer
    next.name = PRAYER_FAJR; // Default: tomorrow's Fajr
    next.startTime = today.fajr;
    next.countdownSeconds = 0;
    next.remainingSeconds = 0;
    next.windowOpen = false;
    next.currentPrayer = PRAYER_NONE;

    for (int i = 0; i < 6; i++) {
        int prayerSec = timeToMinutes(prayers[i].time) * 60;
        if (prayerSec > nowSec) {
            // Skip sunrise - it's not a prayer
            if (prayers[i].name == PRAYER_SUNRISE) continue;

            next.name = prayers[i].name;
            next.startTime = prayers[i].time;
            next.countdownSeconds = prayerSec - nowSec;
            break;
        }
    }

    // Determine current prayer window
    // Fajr ends at Sunrise, Dhuhr ends at Asr, Asr ends at Maghrib,
    // Maghrib ends at Isha, Isha ends at Fajr (next day)
    PrayerTime windowEnds[] = {
        today.sunrise,   // Fajr window ends
        {0, 0},          // Sunrise - not a window
        today.asr,       // Dhuhr window ends
        today.maghrib,   // Asr window ends
        today.isha,      // Maghrib window ends
        today.fajr       // Isha window ends (next day Fajr)
    };

    for (int i = 5; i >= 0; i--) {
        if (prayers[i].name == PRAYER_SUNRISE) continue;
        int startSec = timeToMinutes(prayers[i].time) * 60;
        int endSec   = timeToMinutes(windowEnds[i]) * 60;

        // Handle Isha -> Fajr crossing midnight
        if (i == 5 && endSec < startSec) endSec += 86400;

        if (nowSec >= startSec && nowSec < endSec) {
            next.windowOpen = true;
            next.currentPrayer = prayers[i].name;
            next.remainingSeconds = endSec - nowSec;
            break;
        }
    }

    // --- Prohibited times ---
    // After sunrise
    int sunriseMin = timeToMinutes(today.sunrise);
    prohibited.afterSunrise.startHour   = today.sunrise.hour;
    prohibited.afterSunrise.startMinute = today.sunrise.minute;
    prohibited.afterSunrise.endHour   = (sunriseMin + settings.sunriseOffset) / 60;
    prohibited.afterSunrise.endMinute = (sunriseMin + settings.sunriseOffset) % 60;
    prohibited.afterSunrise.active      = (nowMin >= sunriseMin && nowMin < sunriseMin + settings.sunriseOffset);

    // At zawal (solar noon ~ Dhuhr)
    int dhuhrMin = timeToMinutes(today.dhuhr);
    int zawalStart = dhuhrMin - settings.zawalOffset;
    prohibited.atZawal.startHour   = zawalStart / 60;
    prohibited.atZawal.startMinute = zawalStart % 60;
    prohibited.atZawal.endHour     = today.dhuhr.hour;
    prohibited.atZawal.endMinute   = today.dhuhr.minute;
    prohibited.atZawal.active      = (nowMin >= zawalStart && nowMin < dhuhrMin);

    // Before sunset (Maghrib)
    int maghribMin = timeToMinutes(today.maghrib);
    int sunsetStart = maghribMin - settings.sunsetOffset;
    prohibited.beforeSunset.startHour   = sunsetStart / 60;
    prohibited.beforeSunset.startMinute = sunsetStart % 60;
    prohibited.beforeSunset.endHour     = today.maghrib.hour;
    prohibited.beforeSunset.endMinute   = today.maghrib.minute;
    prohibited.beforeSunset.active      = (nowMin >= sunsetStart && nowMin < maghribMin);

    // Find next upcoming prohibited window
    prohibited.nextWindow = {0, 0, 0, 0, false};
    if (prohibited.afterSunrise.active) {
        prohibited.nextWindow = prohibited.afterSunrise;
    } else if (prohibited.atZawal.active) {
        prohibited.nextWindow = prohibited.atZawal;
    } else if (prohibited.beforeSunset.active) {
        prohibited.nextWindow = prohibited.beforeSunset;
    } else if (nowMin < sunriseMin) {
        prohibited.nextWindow = prohibited.afterSunrise;
    } else if (nowMin < zawalStart) {
        prohibited.nextWindow = prohibited.atZawal;
    } else if (nowMin < sunsetStart) {
        prohibited.nextWindow = prohibited.beforeSunset;
    }
}

bool isAzanTime(const DailyPrayers& prayers) {
    struct tm t;
    if (!getLocalTime(&t)) return false;

    int nowMin = t.tm_hour * 60 + t.tm_min;
    int day = t.tm_mday;

    PrayerTime times[] = {prayers.fajr, prayers.dhuhr, prayers.asr, prayers.maghrib, prayers.isha};
    PrayerName names[] = {PRAYER_FAJR, PRAYER_DHUHR, PRAYER_ASR, PRAYER_MAGHRIB, PRAYER_ISHA};

    for (int i = 0; i < 5; i++) {
        int prayerMin = timeToMinutes(times[i]);
        // Trigger within a 1-minute window and only once per day per prayer
        if (nowMin == prayerMin && t.tm_sec < 30) {
            if (lastAzanDay != day || lastAzanPrayer != names[i]) {
                lastAzanPrayer = names[i];
                lastAzanDay = day;
                return true;
            }
        }
    }
    return false;
}

PrayerName getCurrentPrayerName() {
    return lastAzanPrayer;
}

const char* getPrayerNameStr(PrayerName name) {
    switch (name) {
        case PRAYER_FAJR:    return "Fajr";
        case PRAYER_SUNRISE: return "Sunrise";
        case PRAYER_DHUHR:   return "Dhuhr";
        case PRAYER_ASR:     return "Asr";
        case PRAYER_MAGHRIB: return "Maghrib";
        case PRAYER_ISHA:    return "Isha";
        default:             return "--";
    }
}

} // namespace PrayerManager
