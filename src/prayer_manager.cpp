#include "prayer_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>

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

    // Parse "HH:MM" string into PrayerTime
    PrayerTime parseTime(const char* str) {
        PrayerTime pt = {0, 0};
        if (str && strlen(str) >= 5) {
            pt.hour = (str[0] - '0') * 10 + (str[1] - '0');
            pt.minute = (str[3] - '0') * 10 + (str[4] - '0');
        }
        return pt;
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
        if (!deserializeJson(doc, json)) {
            cachedMonth = doc["month"] | -1;
            cachedYear  = doc["year"]  | -1;

            int m = currentMonth();
            int y = currentYear();

            if (cachedMonth == m && cachedYear == y) {
                JsonArray days = doc["days"].as<JsonArray>();
                cachedDaysCount = 0;
                for (JsonObject day : days) {
                    if (cachedDaysCount >= 31) break;
                    cachedPrayers[cachedDaysCount].fajr    = parseTime(day["fajr"]);
                    cachedPrayers[cachedDaysCount].sunrise  = parseTime(day["sunrise"]);
                    cachedPrayers[cachedDaysCount].dhuhr   = parseTime(day["dhuhr"]);
                    cachedPrayers[cachedDaysCount].asr     = parseTime(day["asr"]);
                    cachedPrayers[cachedDaysCount].maghrib = parseTime(day["maghrib"]);
                    cachedPrayers[cachedDaysCount].isha    = parseTime(day["isha"]);
                    cachedDaysCount++;
                }
                Serial.printf("[PRAYER] Loaded %d days from cache.\n", cachedDaysCount);
                return cachedDaysCount > 0;
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
    cachedDaysCount = 0;

    // Build cache document
    JsonDocument cacheDoc;
    cacheDoc["month"] = month;
    cacheDoc["year"]  = year;
    JsonArray cacheDays = cacheDoc["days"].to<JsonArray>();

    for (JsonObject dayObj : data) {
        if (cachedDaysCount >= 31) break;

        JsonObject timings = dayObj["timings"];
        JsonObject hijri = dayObj["date"]["hijri"];

        // Aladhan returns times like "05:23 (EET)" - extract just HH:MM
        String fajrStr    = String((const char*)timings["Fajr"]).substring(0, 5);
        String sunriseStr = String((const char*)timings["Sunrise"]).substring(0, 5);
        String dhuhrStr   = String((const char*)timings["Dhuhr"]).substring(0, 5);
        String asrStr     = String((const char*)timings["Asr"]).substring(0, 5);
        String maghribStr = String((const char*)timings["Maghrib"]).substring(0, 5);
        String ishaStr    = String((const char*)timings["Isha"]).substring(0, 5);

        cachedPrayers[cachedDaysCount].fajr    = parseTime(fajrStr.c_str());
        cachedPrayers[cachedDaysCount].sunrise  = parseTime(sunriseStr.c_str());
        cachedPrayers[cachedDaysCount].dhuhr   = parseTime(dhuhrStr.c_str());
        cachedPrayers[cachedDaysCount].asr     = parseTime(asrStr.c_str());
        cachedPrayers[cachedDaysCount].maghrib = parseTime(maghribStr.c_str());
        cachedPrayers[cachedDaysCount].isha    = parseTime(ishaStr.c_str());

        JsonObject cacheDay = cacheDays.add<JsonObject>();
        cacheDay["fajr"]    = fajrStr;
        cacheDay["sunrise"] = sunriseStr;
        cacheDay["dhuhr"]   = dhuhrStr;
        cacheDay["asr"]     = asrStr;
        cacheDay["maghrib"] = maghribStr;
        cacheDay["isha"]    = ishaStr;

        // Save Hijri info to cache
        JsonObject cacheHijri = cacheDay["hijri"].to<JsonObject>();
        cacheHijri["day"] = hijri["day"].as<int>();
        cacheHijri["month"] = hijri["month"]["number"].as<int>();
        cacheHijri["monthName"] = hijri["month"]["en"].as<String>();
        cacheHijri["year"] = hijri["year"].as<int>();

        cachedDaysCount++;
    }

    cachedMonth = month;
    cachedYear  = year;

#include "cache_manager.h"
    // Save to flash
    String cacheJson;
    serializeJson(cacheDoc, cacheJson);
    StorageManager::writeFile(PRAYER_CACHE_PATH, cacheJson);
    CacheManager::markUpdated(PRAYER_CACHE_PATH);

    Serial.printf("[PRAYER] Cached %d days for %d/%d.\n", cachedDaysCount, month, year);
    return true;
}

void getTodayPrayers(DailyPrayers& prayers) {
    int day = currentDay();
    if (day >= 1 && day <= cachedDaysCount) {
        prayers = cachedPrayers[day - 1];
    }
}

void updateNextPrayer(NextPrayerInfo& next, ProhibitedTimes& prohibited) {
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
    prohibited.afterSunrise.endHour     = (sunriseMin + DEFAULT_SUNRISE_OFFSET) / 60;
    prohibited.afterSunrise.endMinute   = (sunriseMin + DEFAULT_SUNRISE_OFFSET) % 60;
    prohibited.afterSunrise.active      = (nowMin >= sunriseMin && nowMin < sunriseMin + DEFAULT_SUNRISE_OFFSET);

    // At zawal (solar noon ~ Dhuhr)
    int dhuhrMin = timeToMinutes(today.dhuhr);
    int zawalStart = dhuhrMin - DEFAULT_ZAWAL_OFFSET;
    prohibited.atZawal.startHour   = zawalStart / 60;
    prohibited.atZawal.startMinute = zawalStart % 60;
    prohibited.atZawal.endHour     = today.dhuhr.hour;
    prohibited.atZawal.endMinute   = today.dhuhr.minute;
    prohibited.atZawal.active      = (nowMin >= zawalStart && nowMin < dhuhrMin);

    // Before sunset (Maghrib)
    int maghribMin = timeToMinutes(today.maghrib);
    int sunsetStart = maghribMin - DEFAULT_SUNSET_OFFSET;
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
