#include "prayer_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <sys/time.h>
#include "cache_manager.h"

// ============================================================
//  PRAYER MANAGER IMPLEMENTATION
//  Fetches prayer times from Aladhan API, caches data,
//  and computes next prayer / prohibited times.
// ============================================================

namespace {
    const DailyPrayers defaultDhakaPrayers = {
        {DEFAULT_DHAKA_FAJR_H,    DEFAULT_DHAKA_FAJR_M},
        {DEFAULT_DHAKA_SUNRISE_H, DEFAULT_DHAKA_SUNRISE_M},
        {DEFAULT_DHAKA_DHUHR_H,   DEFAULT_DHAKA_DHUHR_M},
        {DEFAULT_DHAKA_ASR_H,     DEFAULT_DHAKA_ASR_M},
        {DEFAULT_DHAKA_MAGHRIB_H, DEFAULT_DHAKA_MAGHRIB_M},
        {DEFAULT_DHAKA_ISHA_H,    DEFAULT_DHAKA_ISHA_M}
    };

    DailyPrayers cachedPrayers[31];  // Up to 31 days in a month
    int cachedMonth = -1;
    int cachedYear = -1;
    int cachedDaysCount = 0;
    DailyPrayers singleTodayPrayers = defaultDhakaPrayers;
    bool hasTodayPrayers = true;
    bool liveSynced = false;
    PrayerName lastAzanPrayer = PRAYER_NONE;
    int lastAzanDay = -1;

    bool parseTime(const char* str, PrayerTime& time) {
        if (!str || strlen(str) < 4) return false;
        int h = 0, m = 0;
        if (sscanf(str, "%d:%d", &h, &m) != 2) return false;
        if (h < 0 || h > 23 || m < 0 || m > 59) return false;
        time = {static_cast<uint8_t>(h), static_cast<uint8_t>(m)};
        return true;
    }

    int timeToMinutes(PrayerTime pt) {
        return pt.hour * 60 + pt.minute;
    }

    int currentTimeMinutes() {
        struct tm t;
        if (!getLocalTime(&t, 50)) return -1;
        return t.tm_hour * 60 + t.tm_min;
    }

    int currentDay() {
        struct tm t;
        if (!getLocalTime(&t, 50)) return -1;
        return t.tm_mday;
    }

    int currentMonth() {
        struct tm t;
        if (!getLocalTime(&t, 50)) return -1;
        return t.tm_mon + 1;
    }

    int currentYear() {
        struct tm t;
        if (!getLocalTime(&t, 50)) return -1;
        return t.tm_year + 1900;
    }

    int currentSeconds() {
        struct tm t;
        if (!getLocalTime(&t, 50)) return -1;
        return t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
    }

    bool fetchTodayTimings(const Settings& settings) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[PRAYER] WiFi not connected. Cannot fetch.");
            return false;
        }

        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        const float lat = (settings.latitude >= -90.0f && settings.latitude <= 90.0f && settings.latitude != 0.0f)
            ? settings.latitude : DEFAULT_LATITUDE;
        const float lon = (settings.longitude >= -180.0f && settings.longitude <= 180.0f && settings.longitude != 0.0f)
            ? settings.longitude : DEFAULT_LONGITUDE;
        const uint8_t method = settings.calcMethod;
        const uint8_t school = settings.asrSchool;

        String url = String(ALADHAN_API_BASE) + "/timings"
            "?latitude=" + String(lat, 4) +
            "&longitude=" + String(lon, 4) +
            "&method=" + String(method) +
            "&school=" + String(school);

        Serial.printf("[PRAYER] Fetching live prayer timings: %s\n", url.c_str());
        if (!http.begin(client, url)) {
            Serial.println("[PRAYER] HTTP begin failed.");
            return false;
        }

        http.setTimeout(15000);
        int code = http.GET();
        if (code != HTTP_CODE_OK) {
            Serial.printf("[PRAYER] HTTP error: %d\n", code);
            http.end();
            return false;
        }

        String payload = http.getString();
        http.end();

        if (payload.isEmpty()) {
            Serial.println("[PRAYER] Empty response payload from Aladhan API.");
            return false;
        }

        JsonDocument doc;
        DeserializationError err = deserializeJson(doc, payload);
        if (err) {
            Serial.printf("[PRAYER] JSON error: %s\n", err.c_str());
            return false;
        }

        JsonObject data = doc["data"].as<JsonObject>();
        if (data.isNull()) {
            Serial.println("[PRAYER] No 'data' object in response.");
            return false;
        }

        JsonObject timings = data["timings"].as<JsonObject>();
        if (timings.isNull()) {
            Serial.println("[PRAYER] No 'timings' in response.");
            return false;
        }

        DailyPrayers fetched;
        const char* fStr = timings["Fajr"];
        const char* sStr = timings["Sunrise"];
        const char* dStr = timings["Dhuhr"];
        const char* aStr = timings["Asr"];
        const char* mStr = timings["Maghrib"];
        const char* iStr = timings["Isha"];

        if (!parseTime(fStr, fetched.fajr) ||
            !parseTime(sStr, fetched.sunrise) ||
            !parseTime(dStr, fetched.dhuhr) ||
            !parseTime(aStr, fetched.asr) ||
            !parseTime(mStr, fetched.maghrib) ||
            !parseTime(iStr, fetched.isha) ||
            fetched.fajr.hour == 0) {
            Serial.println("[PRAYER] Could not parse valid non-zero prayer times from API response.");
            return false;
        }

        singleTodayPrayers = fetched;
        hasTodayPrayers = true;
        liveSynced = true;

        // Auto-set device clock from Aladhan trusted server timestamp if clock is not synced
        const uint32_t ts = data["date"]["timestamp"] | 0;
        if (ts > 1700000000UL && time(nullptr) < 1700000000UL) {
            timeval tv = { static_cast<time_t>(ts), 0 };
            settimeofday(&tv, nullptr);
            Serial.printf("[PRAYER] Synced device clock from Aladhan API timestamp: %u\n", ts);
        }

        const int day = data["date"]["gregorian"]["day"] | (currentDay() > 0 ? currentDay() : 1);
        if (day >= 1 && day <= 31) {
            cachedPrayers[day - 1] = fetched;
            if (cachedDaysCount < day) cachedDaysCount = day;
        }

        // Save today's timing & Hijri info to LittleFS cache for offline resilience
        JsonDocument cacheDoc;
        cacheDoc["day"] = day;
        cacheDoc["fajr"] = String(fStr).substring(0, 5);
        cacheDoc["sunrise"] = String(sStr).substring(0, 5);
        cacheDoc["dhuhr"] = String(dStr).substring(0, 5);
        cacheDoc["asr"] = String(aStr).substring(0, 5);
        cacheDoc["maghrib"] = String(mStr).substring(0, 5);
        cacheDoc["isha"] = String(iStr).substring(0, 5);

        JsonObject hijri = data["date"]["hijri"];
        if (!hijri.isNull()) {
            JsonObject cacheHijri = cacheDoc["hijri"].to<JsonObject>();
            cacheHijri["day"] = hijri["day"] | 1;
            cacheHijri["month"] = hijri["month"]["number"] | 1;
            cacheHijri["monthName"] = hijri["month"]["en"] | "Muharram";
            cacheHijri["year"] = hijri["year"] | 1448;
        }

        String cacheJson;
        if (serializeJson(cacheDoc, cacheJson) > 0) {
            StorageManager::writeFile(PRAYER_CACHE_PATH, cacheJson);
            CacheManager::markUpdated(PRAYER_CACHE_PATH);
        }

        Serial.printf("[PRAYER] Live times ready: Fajr %02u:%02u, Sunrise %02u:%02u, Dhuhr %02u:%02u, Asr %02u:%02u, Maghrib %02u:%02u, Isha %02u:%02u\n",
                      fetched.fajr.hour, fetched.fajr.minute,
                      fetched.sunrise.hour, fetched.sunrise.minute,
                      fetched.dhuhr.hour, fetched.dhuhr.minute,
                      fetched.asr.hour, fetched.asr.minute,
                      fetched.maghrib.hour, fetched.maghrib.minute,
                      fetched.isha.hour, fetched.isha.minute);

        return true;
    }
}

namespace PrayerManager {

bool init(const Settings& settings) {
    // 1. Initialize with default Dhaka prayer times immediately so UI is never 00:00
    singleTodayPrayers = defaultDhakaPrayers;
    hasTodayPrayers = true;

    // 2. Try loading from LittleFS cache
    String json = StorageManager::readFile(PRAYER_CACHE_PATH);
    if (!json.isEmpty()) {
        JsonDocument doc;
        if (!deserializeJson(doc, json)) {
            // Check monthly calendar format
            if (doc["days"].is<JsonArray>()) {
                JsonArray days = doc["days"].as<JsonArray>();
                int loadedCount = 0;
                for (JsonObject day : days) {
                    if (loadedCount >= 31) break;
                    DailyPrayers& prayer = cachedPrayers[loadedCount];
                    if (parseTime(day["fajr"], prayer.fajr) &&
                        parseTime(day["sunrise"], prayer.sunrise) &&
                        parseTime(day["dhuhr"], prayer.dhuhr) &&
                        parseTime(day["asr"], prayer.asr) &&
                        parseTime(day["maghrib"], prayer.maghrib) &&
                        parseTime(day["isha"], prayer.isha)) {
                        loadedCount++;
                    } else {
                        break;
                    }
                }
                if (loadedCount >= 28) {
                    cachedDaysCount = loadedCount;
                    cachedMonth = doc["month"] | -1;
                    cachedYear = doc["year"] | -1;
                    const int d = currentDay();
                    if (d >= 1 && d <= cachedDaysCount) {
                        singleTodayPrayers = cachedPrayers[d - 1];
                    } else {
                        singleTodayPrayers = cachedPrayers[0];
                    }
                    Serial.printf("[PRAYER] Loaded %d days from monthly cache.\n", cachedDaysCount);
                }
            } else if (doc["fajr"].is<const char*>()) {
                // Single day cache format
                DailyPrayers loaded;
                if (parseTime(doc["fajr"], loaded.fajr) &&
                    parseTime(doc["sunrise"], loaded.sunrise) &&
                    parseTime(doc["dhuhr"], loaded.dhuhr) &&
                    parseTime(doc["asr"], loaded.asr) &&
                    parseTime(doc["maghrib"], loaded.maghrib) &&
                    parseTime(doc["isha"], loaded.isha)) {
                    singleTodayPrayers = loaded;
                    Serial.println("[PRAYER] Loaded today's prayers from daily cache.");
                }
            }
        }
    }

    // 3. If already connected to WiFi, fetch live timings now
    if (WiFi.status() == WL_CONNECTED) {
        fetchTodayTimings(settings);
    }

    return true;
}

bool refresh(const Settings& settings) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[PRAYER] WiFi not connected. Cannot refresh.");
        return hasTodayPrayers;
    }

    return fetchTodayTimings(settings);
}

void getTodayPrayers(DailyPrayers& prayers) {
    const int day = currentDay();
    if (day >= 1 && day <= cachedDaysCount && cachedPrayers[day - 1].fajr.hour > 0) {
        prayers = cachedPrayers[day - 1];
    } else if (hasTodayPrayers && singleTodayPrayers.fajr.hour > 0) {
        prayers = singleTodayPrayers;
    } else {
        prayers = defaultDhakaPrayers;
    }

    // Safeguard: Ensure no prayer time is ever 00:00
    if (prayers.fajr.hour == 0 && prayers.fajr.minute == 0)       prayers.fajr    = defaultDhakaPrayers.fajr;
    if (prayers.sunrise.hour == 0 && prayers.sunrise.minute == 0) prayers.sunrise = defaultDhakaPrayers.sunrise;
    if (prayers.dhuhr.hour == 0 && prayers.dhuhr.minute == 0)     prayers.dhuhr   = defaultDhakaPrayers.dhuhr;
    if (prayers.asr.hour == 0 && prayers.asr.minute == 0)         prayers.asr     = defaultDhakaPrayers.asr;
    if (prayers.maghrib.hour == 0 && prayers.maghrib.minute == 0) prayers.maghrib = defaultDhakaPrayers.maghrib;
    if (prayers.isha.hour == 0 && prayers.isha.minute == 0)       prayers.isha    = defaultDhakaPrayers.isha;
}

void updateNextPrayer(NextPrayerInfo& next, ProhibitedTimes& prohibited,
                      const Settings& settings) {
    DailyPrayers today;
    getTodayPrayers(today);

    // Safeguards for prohibited-time window offsets (defaults: 15, 10, 15)
    const uint8_t sOff = settings.sunriseOffset > 0 ? settings.sunriseOffset : DEFAULT_SUNRISE_OFFSET;
    const uint8_t zOff = settings.zawalOffset > 0   ? settings.zawalOffset   : DEFAULT_ZAWAL_OFFSET;
    const uint8_t mOff = settings.sunsetOffset > 0  ? settings.sunsetOffset  : DEFAULT_SUNSET_OFFSET;

    // --- Prohibited times (Calculated directly from today's Sunrise, Dhuhr, and Maghrib) ---
    // 1. After sunrise: [Sunrise, Sunrise + sunriseOffset]
    const int sunriseMin = timeToMinutes(today.sunrise);
    prohibited.afterSunrise.startHour   = today.sunrise.hour;
    prohibited.afterSunrise.startMinute = today.sunrise.minute;
    const int afterSunriseEnd = sunriseMin + sOff;
    prohibited.afterSunrise.endHour   = (afterSunriseEnd / 60) % 24;
    prohibited.afterSunrise.endMinute = afterSunriseEnd % 60;

    // 2. At zawal (solar noon before Dhuhr): [Dhuhr - zawalOffset, Dhuhr]
    const int dhuhrMin = timeToMinutes(today.dhuhr);
    int zawalStart = dhuhrMin - zOff;
    if (zawalStart < 0) zawalStart += 1440;
    prohibited.atZawal.startHour   = (zawalStart / 60) % 24;
    prohibited.atZawal.startMinute = zawalStart % 60;
    prohibited.atZawal.endHour     = today.dhuhr.hour;
    prohibited.atZawal.endMinute   = today.dhuhr.minute;

    // 3. Before sunset (before Maghrib): [Maghrib - sunsetOffset, Maghrib]
    const int maghribMin = timeToMinutes(today.maghrib);
    int sunsetStart = maghribMin - mOff;
    if (sunsetStart < 0) sunsetStart += 1440;
    prohibited.beforeSunset.startHour   = (sunsetStart / 60) % 24;
    prohibited.beforeSunset.startMinute = sunsetStart % 60;
    prohibited.beforeSunset.endHour     = today.maghrib.hour;
    prohibited.beforeSunset.endMinute   = today.maghrib.minute;

    const int nowMin = currentTimeMinutes();
    const int nowSec = currentSeconds();

    if (nowMin >= 0) {
        prohibited.afterSunrise.active = (nowMin >= sunriseMin && nowMin < afterSunriseEnd);
        prohibited.atZawal.active      = (nowMin >= zawalStart && nowMin < dhuhrMin);
        prohibited.beforeSunset.active = (nowMin >= sunsetStart && nowMin < maghribMin);
    } else {
        prohibited.afterSunrise.active = false;
        prohibited.atZawal.active      = false;
        prohibited.beforeSunset.active = false;
    }

    // Determine soonest upcoming or active prohibited window
    prohibited.nextWindow = {0, 0, 0, 0, false};
    if (prohibited.afterSunrise.active) {
        prohibited.nextWindow = prohibited.afterSunrise;
    } else if (prohibited.atZawal.active) {
        prohibited.nextWindow = prohibited.atZawal;
    } else if (prohibited.beforeSunset.active) {
        prohibited.nextWindow = prohibited.beforeSunset;
    } else if (nowMin >= 0) {
        if (nowMin < sunriseMin) {
            prohibited.nextWindow = prohibited.afterSunrise;
        } else if (nowMin < zawalStart) {
            prohibited.nextWindow = prohibited.atZawal;
        } else if (nowMin < sunsetStart) {
            prohibited.nextWindow = prohibited.beforeSunset;
        } else {
            // After sunset, next prohibited window is tomorrow's after-sunrise
            prohibited.nextWindow = prohibited.afterSunrise;
        }
    } else {
        prohibited.nextWindow = prohibited.afterSunrise;
    }

    // --- Next prayer & countdown ---
    struct { PrayerName name; PrayerTime time; } prayerList[] = {
        {PRAYER_FAJR,    today.fajr},
        {PRAYER_SUNRISE, today.sunrise},
        {PRAYER_DHUHR,   today.dhuhr},
        {PRAYER_ASR,     today.asr},
        {PRAYER_MAGHRIB, today.maghrib},
        {PRAYER_ISHA,    today.isha}
    };

    next.name = PRAYER_FAJR;
    next.startTime = today.fajr;
    next.countdownSeconds = 0;
    next.remainingSeconds = 0;
    next.windowOpen = false;
    next.currentPrayer = PRAYER_NONE;

    if (nowSec >= 0) {
        bool foundUpcoming = false;
        for (int i = 0; i < 6; i++) {
            const int pSec = timeToMinutes(prayerList[i].time) * 60;
            if (pSec > nowSec) {
                if (prayerList[i].name == PRAYER_SUNRISE) continue;
                next.name = prayerList[i].name;
                next.startTime = prayerList[i].time;
                next.countdownSeconds = pSec - nowSec;
                foundUpcoming = true;
                break;
            }
        }
        if (!foundUpcoming) {
            // After Isha -> next prayer is tomorrow's Fajr
            next.name = PRAYER_FAJR;
            next.startTime = today.fajr;
            const int fajrSec = timeToMinutes(today.fajr) * 60;
            next.countdownSeconds = (86400 - nowSec) + fajrSec;
        }

        // Current prayer window
        PrayerTime windowEnds[] = {
            today.sunrise,   // Fajr ends at Sunrise
            {0, 0},          // Sunrise
            today.asr,       // Dhuhr ends at Asr
            today.maghrib,   // Asr ends at Maghrib
            today.isha,      // Maghrib ends at Isha
            today.fajr       // Isha ends at Fajr (next day)
        };

        for (int i = 5; i >= 0; i--) {
            if (prayerList[i].name == PRAYER_SUNRISE) continue;
            const int startSec = timeToMinutes(prayerList[i].time) * 60;
            int endSec   = timeToMinutes(windowEnds[i]) * 60;

            if (i == 5 && endSec < startSec) endSec += 86400;

            if (nowSec >= startSec && nowSec < endSec) {
                next.windowOpen = true;
                next.currentPrayer = prayerList[i].name;
                next.remainingSeconds = endSec - nowSec;
                break;
            }
        }
    }
}

bool isAzanTime(const DailyPrayers& prayers) {
    struct tm t;
    if (!getLocalTime(&t, 50)) return false;

    const int nowMin = t.tm_hour * 60 + t.tm_min;
    const int day = t.tm_mday;

    PrayerTime times[] = {prayers.fajr, prayers.dhuhr, prayers.asr, prayers.maghrib, prayers.isha};
    PrayerName names[] = {PRAYER_FAJR, PRAYER_DHUHR, PRAYER_ASR, PRAYER_MAGHRIB, PRAYER_ISHA};

    for (int i = 0; i < 5; i++) {
        const int prayerMin = timeToMinutes(times[i]);
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

bool isLiveSynced() {
    return liveSynced;
}

} // namespace PrayerManager
