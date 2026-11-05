#include "managers/prayer/prayer_manager.h"
#include <prayer_bot_config.h>
#include "network/https_client.h"
#include "managers/storage/storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <limits.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include "managers/storage/cache_manager.h"

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
        {DEFAULT_DHAKA_ISHA_H,    DEFAULT_DHAKA_ISHA_M},
        {DEFAULT_DHAKA_FAJR_H,    DEFAULT_DHAKA_FAJR_M}
    };

    struct CachedPrayerDay {
        int year;
        int month;
        int day;
        DailyPrayers prayers;
    };

    void setCacheDay(const char* date, DailyPrayers& prayers);

    DailyPrayers cachedPrayers[31];  // Up to 31 days in a month
    CachedPrayerDay backupPrayers[PRAYER_CACHE_BACKUP_DAYS] = {};
    int backupDaysCount = 0;
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

    int jsonInteger(JsonVariantConst value, int fallback) {
        if (value.is<int>()) return value.as<int>();
        const char* text = value.as<const char*>();
        if (!text || !text[0]) return fallback;
        char* end = nullptr;
        const long parsed = strtol(text, &end, 10);
        return end != text && *end == '\0' &&
            parsed >= INT_MIN && parsed <= INT_MAX
                ? static_cast<int>(parsed) : fallback;
    }

    bool parseGregorianDate(JsonObjectConst day, int& year, int& month,
                            int& date) {
        const char* formattedDate = day["date"]["gregorian"]["date"] | "";
        int parsedDay = 0;
        int parsedMonth = 0;
        int parsedYear = 0;
        if (sscanf(formattedDate, "%2d-%2d-%4d", &parsedDay, &parsedMonth,
                   &parsedYear) != 3 ||
            parsedYear < 2000 || parsedMonth < 1 || parsedMonth > 12 ||
            parsedDay < 1 || parsedDay > 31) {
            return false;
        }
        year = parsedYear;
        month = parsedMonth;
        date = parsedDay;
        return true;
    }

    bool fetchCalendarMonth(const Settings& settings, WiFiClientSecure& client,
                           int year, int month, JsonDocument& document) {
        HTTPClient http;
        const float lat = (settings.latitude >= -90.0f && settings.latitude <= 90.0f && settings.latitude != 0.0f)
            ? settings.latitude : DEFAULT_LATITUDE;
        const float lon = (settings.longitude >= -180.0f && settings.longitude <= 180.0f && settings.longitude != 0.0f)
            ? settings.longitude : DEFAULT_LONGITUDE;
        const String url = String(ALADHAN_API_BASE) + "/calendar/" +
            String(year) + "/" + String(month) +
            "?latitude=" + String(lat, 4) +
            "&longitude=" + String(lon, 4) +
            "&method=" + String(settings.calcMethod) +
            "&school=" + String(settings.asrSchool);
        if (!http.begin(client, url)) {
            Serial.println("[PRAYER] Could not start calendar backup request.");
            return false;
        }
        http.setTimeout(15000);
        const int code = http.GET();
        if (code != HTTP_CODE_OK) {
            Serial.printf("[PRAYER] Calendar backup HTTP error: %d\n", code);
            http.end();
            return false;
        }
        JsonDocument filter;
        JsonArray filteredDays = filter["data"].to<JsonArray>();
        JsonObject day = filteredDays.add<JsonObject>();
        day["date"]["gregorian"]["date"] = true;
        day["date"]["hijri"]["day"] = true;
        day["date"]["hijri"]["month"]["number"] = true;
        day["date"]["hijri"]["month"]["en"] = true;
        day["date"]["hijri"]["year"] = true;
        const char* const timeFields[] = {
            "Fajr", "Imsak", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"
        };
        for (const char* field : timeFields)
            day["timings"][field] = true;

        const DeserializationError error = deserializeJson(
            document, http.getStream(), DeserializationOption::Filter(filter));
        http.end();
        if (error || !document["data"].is<JsonArray>()) {
            Serial.printf("[PRAYER] Invalid calendar backup response: %s\n",
                          error ? error.c_str() : "data is not an array");
            return false;
        }
        return true;
    }

    bool appendCalendarDay(JsonArray days, JsonObjectConst source) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parseGregorianDate(source, year, month, day)) return false;
        JsonObjectConst timings = source["timings"];
        const char* const timeFields[] = {
            "Fajr", "Imsak", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"
        };
        PrayerTime parsedTimes[7];
        for (size_t i = 0; i < 7; ++i) {
            const char* value = timings[timeFields[i]] | "";
            if (i == 1 && !value[0]) {
                if (!parseTime(timings["Fajr"], parsedTimes[i])) return false;
            } else if (!parseTime(value, parsedTimes[i])) {
                if (i != 1) return false;
                if (!parseTime(timings["Fajr"], parsedTimes[i])) return false;
            }
        }

        JsonObject cached = days.add<JsonObject>();
        if (cached.isNull()) return false;
        cached["date"] = String(year) + "-" +
            (month < 10 ? "0" : "") + String(month) + "-" +
            (day < 10 ? "0" : "") + String(day);
        cached["fetchedAt"] = static_cast<uint32_t>(time(nullptr));
        const char* const cachedFields[] = {
            "fajr", "imsak", "sunrise", "dhuhr", "asr", "maghrib", "isha"
        };
        for (size_t i = 0; i < 7; ++i) {
            const char* value = timings[timeFields[i]] | "";
            cached[cachedFields[i]] = value[0]
                ? String(value).substring(0, 5)
                : String(timings["Fajr"] | "").substring(0, 5);
        }
        char cacheDate[11];
        snprintf(cacheDate, sizeof(cacheDate), "%04d-%02d-%02d",
                 year, month, day);
        DailyPrayers cachedPrayers = {
            parsedTimes[0], parsedTimes[2], parsedTimes[3], parsedTimes[4],
            parsedTimes[5], parsedTimes[6], parsedTimes[1]
        };
        setCacheDay(cacheDate, cachedPrayers);

        JsonObjectConst hijri = source["date"]["hijri"];
        if (!hijri.isNull()) {
            JsonObject cachedHijri = cached["hijri"].to<JsonObject>();
            cachedHijri["day"] = jsonInteger(hijri["day"], 1);
            cachedHijri["month"] = jsonInteger(hijri["month"]["number"], 1);
            cachedHijri["monthName"] = hijri["month"]["en"] | "Muharram";
            cachedHijri["year"] = jsonInteger(hijri["year"], 1448);
        }
        return true;
    }

    int timeToMinutes(PrayerTime pt) {
        return pt.hour * 60 + pt.minute;
    }

    int currentDay() {
        struct tm t;
        if (!getLocalTime(&t, 0) || t.tm_year < (2020 - 1900)) return -1;
        return t.tm_mday;
    }

    bool getCachedPrayersForDate(const struct tm& date, DailyPrayers& prayers) {
        const int year = date.tm_year + 1900;
        const int month = date.tm_mon + 1;
        for (int i = 0; i < backupDaysCount; ++i) {
            if (backupPrayers[i].year == year &&
                backupPrayers[i].month == month &&
                backupPrayers[i].day == date.tm_mday) {
                prayers = backupPrayers[i].prayers;
                return true;
            }
        }
        if (cachedYear == year && cachedMonth == month &&
            date.tm_mday >= 1 && date.tm_mday <= cachedDaysCount) {
            prayers = cachedPrayers[date.tm_mday - 1];
            return true;
        }
        return false;
    }

    void getTodayPrayersForDay(int day, DailyPrayers& prayers) {
        struct tm today = {};
        const bool hasDate = getLocalTime(&today, 0) &&
            today.tm_year >= (2020 - 1900);
        bool foundBackup = false;
        if (hasDate && day == today.tm_mday) {
            for (int i = 0; i < backupDaysCount; ++i) {
                if (backupPrayers[i].year == today.tm_year + 1900 &&
                    backupPrayers[i].month == today.tm_mon + 1 &&
                    backupPrayers[i].day == today.tm_mday) {
                    prayers = backupPrayers[i].prayers;
                    foundBackup = true;
                    break;
                }
            }
        } else if (!hasDate && hasTodayPrayers) {
            prayers = singleTodayPrayers;
            foundBackup = true;
        }

        if (!foundBackup && cachedMonth == (hasDate ? today.tm_mon + 1 : -1) &&
            cachedYear == (hasDate ? today.tm_year + 1900 : -1) &&
            day >= 1 && day <= cachedDaysCount &&
            cachedPrayers[day - 1].fajr.hour > 0) {
            prayers = cachedPrayers[day - 1];
        } else if (!foundBackup && backupDaysCount == 0 &&
                   hasTodayPrayers && singleTodayPrayers.fajr.hour > 0) {
            prayers = singleTodayPrayers;
        } else if (!foundBackup) {
            prayers = defaultDhakaPrayers;
        }

        if (prayers.fajr.hour == 0 && prayers.fajr.minute == 0)
            prayers.fajr = defaultDhakaPrayers.fajr;
        if (prayers.sunrise.hour == 0 && prayers.sunrise.minute == 0)
            prayers.sunrise = defaultDhakaPrayers.sunrise;
        if (prayers.dhuhr.hour == 0 && prayers.dhuhr.minute == 0)
            prayers.dhuhr = defaultDhakaPrayers.dhuhr;
        if (prayers.asr.hour == 0 && prayers.asr.minute == 0)
            prayers.asr = defaultDhakaPrayers.asr;
        if (prayers.maghrib.hour == 0 && prayers.maghrib.minute == 0)
            prayers.maghrib = defaultDhakaPrayers.maghrib;
        if (prayers.isha.hour == 0 && prayers.isha.minute == 0)
            prayers.isha = defaultDhakaPrayers.isha;
        if (prayers.imsak.hour == 0 && prayers.imsak.minute == 0)
            prayers.imsak = prayers.fajr;
    }

    void setCacheDay(const char* date, DailyPrayers& prayers) {
        int year = 0;
        int month = 0;
        int day = 0;
        if (!date || sscanf(date, "%4d-%2d-%2d", &year, &month, &day) != 3 ||
            year < 2000 || month < 1 || month > 12 || day < 1 || day > 31) {
            return;
        }

        int existingIndex = -1;
        for (int i = 0; i < backupDaysCount; ++i) {
            if (backupPrayers[i].year == year &&
                backupPrayers[i].month == month &&
                backupPrayers[i].day == day) {
                existingIndex = i;
                break;
            }
        }
        if (existingIndex >= 0) {
            for (int i = existingIndex; i > 0; --i) {
                backupPrayers[i] = backupPrayers[i - 1];
            }
        } else {
            for (int i = backupDaysCount; i > 0; --i) {
                if (i < PRAYER_CACHE_BACKUP_DAYS)
                    backupPrayers[i] = backupPrayers[i - 1];
            }
        }
        backupPrayers[0] = {year, month, day, prayers};
        if (existingIndex < 0 && backupDaysCount < PRAYER_CACHE_BACKUP_DAYS)
            ++backupDaysCount;
    }

    void copyCachedDay(JsonObject destination, JsonObjectConst source) {
        destination["date"] = source["date"];
        destination["fetchedAt"] = source["fetchedAt"];
        const char* const timeFields[] = {
            "fajr", "imsak", "sunrise", "dhuhr", "asr", "maghrib", "isha"
        };
        for (const char* field : timeFields) destination[field] = source[field];
        if (source["hijri"].is<JsonObjectConst>()) {
            JsonObject hijri = destination["hijri"].to<JsonObject>();
            hijri["day"] = source["hijri"]["day"];
            hijri["month"] = source["hijri"]["month"];
            hijri["monthName"] = source["hijri"]["monthName"];
            hijri["year"] = source["hijri"]["year"];
        }
    }

    int currentSeconds(const struct tm& time) {
        return time.tm_hour * 3600 + time.tm_min * 60 + time.tm_sec;
    }

    bool fetchTodayTimings(const Settings& settings) {
        if (WiFi.status() != WL_CONNECTED) {
            Serial.println("[PRAYER] WiFi not connected. Cannot fetch.");
            return false;
        }

        WiFiClientSecure client;
        configureHttpsClient(client);

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

        JsonDocument doc;
        JsonDocument filter;
        JsonObject filteredData = filter["data"].to<JsonObject>();
        filteredData["date"]["timestamp"] = true;
        filteredData["date"]["gregorian"]["date"] = true;
        filteredData["date"]["hijri"]["day"] = true;
        filteredData["date"]["hijri"]["month"]["number"] = true;
        filteredData["date"]["hijri"]["month"]["en"] = true;
        filteredData["date"]["hijri"]["year"] = true;
        const char* const filteredTimeFields[] = {
            "Fajr", "Imsak", "Sunrise", "Dhuhr", "Asr", "Maghrib", "Isha"
        };
        for (const char* field : filteredTimeFields)
            filteredData["timings"][field] = true;
        DeserializationError err = deserializeJson(
            doc, http.getStream(), DeserializationOption::Filter(filter));
        http.end();
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
        const char* imsakStr = timings["Imsak"];
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
        if (!parseTime(imsakStr, fetched.imsak)) fetched.imsak = fetched.fajr;

        singleTodayPrayers = fetched;
        hasTodayPrayers = true;
        liveSynced = true;

        // Auto-set device clock from Aladhan trusted server timestamp if clock is not synced
        const uint32_t ts = static_cast<uint32_t>(
            strtoul(data["date"]["timestamp"] | "0", nullptr, 10));
        if (ts > 1700000000UL && time(nullptr) < 1700000000UL) {
            timeval tv = { static_cast<time_t>(ts), 0 };
            settimeofday(&tv, nullptr);
            Serial.printf("[PRAYER] Synced device clock from Aladhan API timestamp: %u\n", ts);
        }

        struct tm localDate = {};
        const bool hasLocalDate = getLocalTime(&localDate, 0);
        int year = 0;
        int month = 0;
        int day = 0;
        if (!parseGregorianDate(data, year, month, day)) {
            day = hasLocalDate ? localDate.tm_mday : 0;
            month = hasLocalDate ? localDate.tm_mon + 1 : 0;
            year = hasLocalDate ? localDate.tm_year + 1900 : 0;
        }
        char cacheDate[11];
        snprintf(cacheDate, sizeof(cacheDate), "%04d-%02d-%02d", year, month, day);
        setCacheDay(cacheDate, fetched);

        JsonDocument previousDoc;
        String previousJson = StorageManager::readFile(PRAYER_CACHE_PATH);
        if (!previousJson.isEmpty() && deserializeJson(previousDoc, previousJson)) {
            Serial.println("[PRAYER] Existing backup cache is invalid; replacing it.");
        }
        previousJson = "";

        JsonDocument cacheDoc;
        cacheDoc["version"] = 2;
        JsonArray days = cacheDoc["days"].to<JsonArray>();
        JsonObject current = days.add<JsonObject>();
        current["date"] = cacheDate;
        current["fetchedAt"] = static_cast<uint32_t>(time(nullptr));
        current["fajr"] = String(fStr).substring(0, 5);
        current["imsak"] = fetched.imsak.hour == fetched.fajr.hour &&
            fetched.imsak.minute == fetched.fajr.minute
                ? String(fStr).substring(0, 5)
                : String(imsakStr).substring(0, 5);
        current["sunrise"] = String(sStr).substring(0, 5);
        current["dhuhr"] = String(dStr).substring(0, 5);
        current["asr"] = String(aStr).substring(0, 5);
        current["maghrib"] = String(mStr).substring(0, 5);
        current["isha"] = String(iStr).substring(0, 5);

        JsonObject hijri = data["date"]["hijri"];
        if (!hijri.isNull()) {
            JsonObject cacheHijri = current["hijri"].to<JsonObject>();
            cacheHijri["day"] = jsonInteger(hijri["day"], 1);
            cacheHijri["month"] = jsonInteger(hijri["month"]["number"], 1);
            cacheHijri["monthName"] = hijri["month"]["en"] | "Muharram";
            cacheHijri["year"] = jsonInteger(hijri["year"], 1448);
        }

        JsonArray oldDays = previousDoc["days"].as<JsonArray>();
        int copiedDays = 1;
        bool calendarLoaded = false;
        if (year >= 2000 && month >= 1 && month <= 12 && day >= 1 && day <= 31) {
            int calendarYear = year;
            int calendarMonth = month;
            for (uint8_t monthRequest = 0;
                 monthRequest < 2 && copiedDays < PRAYER_CACHE_BACKUP_DAYS;
                 ++monthRequest) {
                JsonDocument calendarDoc;
                if (!fetchCalendarMonth(settings, client, calendarYear,
                                        calendarMonth, calendarDoc)) {
                    break;
                }
                JsonArrayConst calendarDays =
                    calendarDoc["data"].as<JsonArrayConst>();
                int startIndex = 0;
                if (monthRequest == 0) {
                    startIndex = -1;
                    for (size_t i = 0; i < calendarDays.size(); ++i) {
                        int entryYear = 0;
                        int entryMonth = 0;
                        int entryDay = 0;
                        if (parseGregorianDate(calendarDays[i], entryYear,
                                               entryMonth, entryDay) &&
                            entryYear == year && entryMonth == month &&
                            entryDay == day) {
                            startIndex = static_cast<int>(i);
                            break;
                        }
                    }
                    if (startIndex < 0) break;
                    calendarLoaded = true;
                    ++startIndex;
                } else {
                    startIndex = 0;
                }

                for (int i = startIndex;
                     i < static_cast<int>(calendarDays.size()) &&
                     copiedDays < PRAYER_CACHE_BACKUP_DAYS; ++i) {
                    JsonObjectConst calendarEntry = calendarDays[i];
                    int entryYear = 0;
                    int entryMonth = 0;
                    int entryDay = 0;
                    if (!parseGregorianDate(calendarEntry, entryYear,
                                            entryMonth, entryDay)) {
                        Serial.println("[PRAYER] Skipping invalid calendar date in backup.");
                        continue;
                    }
                    char entryDate[11];
                    snprintf(entryDate, sizeof(entryDate), "%04d-%02d-%02d",
                             entryYear, entryMonth, entryDay);
                    bool duplicate = false;
                    for (JsonObject existing : days) {
                        if (strcmp(existing["date"] | "", entryDate) == 0) {
                            duplicate = true;
                            break;
                        }
                    }
                    if (duplicate) continue;
                    if (appendCalendarDay(days, calendarEntry)) {
                        ++copiedDays;
                    } else {
                        Serial.printf("[PRAYER] Could not parse calendar timings for %s.\n",
                                      entryDate);
                    }
                }

                if (copiedDays >= PRAYER_CACHE_BACKUP_DAYS) break;
                if (calendarMonth == 12) {
                    calendarMonth = 1;
                    ++calendarYear;
                } else {
                    ++calendarMonth;
                }
            }
        }

        if (calendarLoaded) {
            Serial.printf("[PRAYER] Cached %d days of current and upcoming prayer times.\n",
                          copiedDays);
            if (copiedDays < PRAYER_CACHE_BACKUP_DAYS) {
                Serial.printf("[PRAYER] Offline prayer cache is partial: %d of %d days.\n",
                              copiedDays, PRAYER_CACHE_BACKUP_DAYS);
            }
        } else if (copiedDays < PRAYER_CACHE_BACKUP_DAYS) {
            Serial.printf("[PRAYER] Could not load the calendar backup; cache contains %d of %d days.\n",
                          copiedDays, PRAYER_CACHE_BACKUP_DAYS);
        }
        char retainedDates[PRAYER_CACHE_BACKUP_DAYS][11] = {};
        if (year >= 2000 && month >= 1 && month <= 12 &&
            day >= 1 && day <= 31) {
            struct tm retainedDate = {};
            retainedDate.tm_year = year - 1900;
            retainedDate.tm_mon = month - 1;
            retainedDate.tm_mday = day;
            for (int i = 0; i < PRAYER_CACHE_BACKUP_DAYS; ++i) {
                if (i > 0) {
                    ++retainedDate.tm_mday;
                    if (mktime(&retainedDate) == static_cast<time_t>(-1))
                        break;
                }
                snprintf(retainedDates[i], sizeof(retainedDates[i]),
                         "%04d-%02d-%02d", retainedDate.tm_year + 1900,
                         retainedDate.tm_mon + 1, retainedDate.tm_mday);
            }
        }
        for (JsonObjectConst oldDay : oldDays) {
            const char* oldDate = oldDay["date"] | "";
            bool retainDate = false;
            for (const char (&date)[11] : retainedDates) {
                if (date[0] && strcmp(oldDate, date) == 0) {
                    retainDate = true;
                    break;
                }
            }
            if (!retainDate || strcmp(oldDate, cacheDate) <= 0) continue;
            bool duplicate = false;
            for (JsonObject existing : days) {
                if (strcmp(existing["date"] | "", oldDate) == 0) {
                    duplicate = true;
                    break;
                }
            }
            if (duplicate) continue;
            if (copiedDays >= PRAYER_CACHE_BACKUP_DAYS) break;
            JsonObject backup = days.add<JsonObject>();
            copyCachedDay(backup, oldDay);
            ++copiedDays;
        }

        String cacheJson;
        if (serializeJson(cacheDoc, cacheJson) > 0) {
            if (StorageManager::writeFile(PRAYER_CACHE_PATH, cacheJson)) {
                CacheManager::markUpdated(PRAYER_CACHE_PATH);
            } else {
                Serial.println("[PRAYER] Could not persist prayer backup cache.");
            }
        } else {
            Serial.println("[PRAYER] Could not serialize prayer backup cache.");
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
        const uint32_t age = CacheManager::getCacheAge(PRAYER_CACHE_PATH);
        const bool fresh = age == 0xFFFFFFFF ||
            age <= CACHE_PRAYER_MAX_DAYS * 86400UL;
        if (!deserializeJson(doc, json) && fresh) {
            JsonArray backupDays = doc["days"].as<JsonArray>();
            if ((doc["version"] | 0) == 2 && !backupDays.isNull()) {
                for (JsonObject day : backupDays) {
                    if (backupDaysCount >= PRAYER_CACHE_BACKUP_DAYS) break;
                    CachedPrayerDay& cached = backupPrayers[backupDaysCount];
                    const char* date = day["date"] | "";
                    DailyPrayers loaded = {};
                    if (sscanf(date, "%4d-%2d-%2d", &cached.year,
                               &cached.month, &cached.day) != 3 ||
                        cached.year < 2000 || cached.month < 1 ||
                        cached.month > 12 || cached.day < 1 ||
                        cached.day > 31 ||
                        !parseTime(day["fajr"], loaded.fajr) ||
                        !parseTime(day["sunrise"], loaded.sunrise) ||
                        !parseTime(day["dhuhr"], loaded.dhuhr) ||
                        !parseTime(day["asr"], loaded.asr) ||
                        !parseTime(day["maghrib"], loaded.maghrib) ||
                        !parseTime(day["isha"], loaded.isha)) {
                        continue;
                    }
                    if (!parseTime(day["imsak"], loaded.imsak)) loaded.imsak = loaded.fajr;
                    cached.prayers = loaded;
                    ++backupDaysCount;
                }
                if (backupDaysCount > 0) {
                    singleTodayPrayers = backupPrayers[0].prayers;
                    struct tm today = {};
                    if (getLocalTime(&today, 0) &&
                        today.tm_year >= (2020 - 1900)) {
                        for (int i = 0; i < backupDaysCount; ++i) {
                            if (backupPrayers[i].year == today.tm_year + 1900 &&
                                backupPrayers[i].month == today.tm_mon + 1 &&
                                backupPrayers[i].day == today.tm_mday) {
                                singleTodayPrayers = backupPrayers[i].prayers;
                                break;
                            }
                        }
                    }
                    Serial.printf("[PRAYER] Loaded %d dated days from offline backup.\n",
                                  backupDaysCount);
                }
            }
            // Check monthly calendar format
            else if (doc["days"].is<JsonArray>()) {
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
                    if (!parseTime(doc["imsak"], loaded.imsak))
                        loaded.imsak = loaded.fajr;
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
    getTodayPrayersForDay(currentDay(), prayers);
}

void updateNextPrayer(NextPrayerInfo& next, ProhibitedTimes& prohibited,
                      const Settings& settings) {
    struct tm currentTime = {};
    const bool timeValid = getLocalTime(&currentTime, 0) &&
        currentTime.tm_year >= (2020 - 1900);
    DailyPrayers today;
    getTodayPrayersForDay(timeValid ? currentTime.tm_mday : -1, today);
    DailyPrayers tomorrow = today;
    if (timeValid) {
        struct tm tomorrowDate = currentTime;
        ++tomorrowDate.tm_mday;
        if (mktime(&tomorrowDate) != static_cast<time_t>(-1)) {
            getCachedPrayersForDate(tomorrowDate, tomorrow);
        }
    }

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

    const int nowMin = timeValid
        ? currentTime.tm_hour * 60 + currentTime.tm_min : -1;
    const int nowSec = timeValid ? currentSeconds(currentTime) : -1;

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
            const int tomorrowSunriseMin = timeToMinutes(tomorrow.sunrise);
            const int tomorrowAfterSunriseEnd = tomorrowSunriseMin + sOff;
            prohibited.nextWindow = {
                tomorrow.sunrise.hour,
                tomorrow.sunrise.minute,
                static_cast<uint8_t>((tomorrowAfterSunriseEnd / 60) % 24),
                static_cast<uint8_t>(tomorrowAfterSunriseEnd % 60),
                false
            };
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
    next.presentPrayer = PRAYER_NONE;
    next.startTime = today.fajr;
    next.endTime = today.sunrise;
    next.upcomingPrayer = PRAYER_FAJR;
    next.upcomingStartTime = today.fajr;
    next.upcomingEndTime = today.sunrise;
    next.countdownSeconds = 0;
    next.remainingSeconds = 0;
    next.windowOpen = false;
    next.currentPrayer = PRAYER_NONE;
    next.currentStartTime = {0, 0};
    next.windowEndTime = {0, 0};

    if (nowSec >= 0) {
        const PrayerName salahNames[] = {
            PRAYER_FAJR, PRAYER_DHUHR, PRAYER_ASR, PRAYER_MAGHRIB, PRAYER_ISHA
        };
        const PrayerTime salahTimes[] = {
            today.fajr, today.dhuhr, today.asr, today.maghrib, today.isha
        };
        next.presentPrayer = PRAYER_ISHA;
        for (size_t i = 0; i < sizeof(salahNames) / sizeof(salahNames[0]); ++i) {
            if (timeToMinutes(salahTimes[i]) * 60 <= nowSec) {
                next.presentPrayer = salahNames[i];
            } else {
                break;
            }
        }

        bool foundUpcoming = false;
        for (int i = 0; i < 6; i++) {
            const int pSec = timeToMinutes(prayerList[i].time) * 60;
            if (pSec > nowSec) {
                if (prayerList[i].name == PRAYER_SUNRISE) continue;
                next.name = prayerList[i].name;
                next.startTime = prayerList[i].time;
                next.upcomingPrayer = prayerList[i].name;
                next.upcomingStartTime = prayerList[i].time;
                next.countdownSeconds = pSec - nowSec;
                foundUpcoming = true;
                break;
            }
        }
        if (!foundUpcoming) {
            // After Isha -> next prayer is tomorrow's Fajr
            next.name = PRAYER_FAJR;
            next.startTime = tomorrow.fajr;
            next.upcomingPrayer = PRAYER_FAJR;
            next.upcomingStartTime = tomorrow.fajr;
            next.upcomingEndTime = tomorrow.sunrise;
            const int fajrSec = timeToMinutes(tomorrow.fajr) * 60;
            next.countdownSeconds = (86400 - nowSec) + fajrSec;
        }

        const PrayerTime ishaWindowEnd =
            nowSec >= timeToMinutes(today.isha) * 60
                ? tomorrow.fajr : today.fajr;
        // Current prayer window
        PrayerTime windowEnds[] = {
            today.sunrise,   // Fajr ends at Sunrise
            {0, 0},          // Sunrise
            today.asr,       // Dhuhr ends at Asr
            today.maghrib,   // Asr ends at Maghrib
            today.isha,      // Maghrib ends at Isha
            ishaWindowEnd    // Isha ends at the next Fajr
        };

        for (int i = 5; i >= 0; i--) {
            if (prayerList[i].name == PRAYER_SUNRISE) continue;
            const int startSec = timeToMinutes(prayerList[i].time) * 60;
            int endSec   = timeToMinutes(windowEnds[i]) * 60;

            if (i == 5 && nowSec < endSec) {
                next.windowOpen = true;
                next.currentPrayer = PRAYER_ISHA;
                next.currentStartTime = prayerList[i].time;
                next.remainingSeconds = endSec - nowSec;
                next.windowEndTime = windowEnds[i];
                break;
            }

            if (i == 5 && endSec < startSec) endSec += 86400;

            if (nowSec >= startSec && nowSec < endSec) {
                next.windowOpen = true;
                next.currentPrayer = prayerList[i].name;
                next.currentStartTime = prayerList[i].time;
                next.remainingSeconds = endSec - nowSec;
                next.windowEndTime = windowEnds[i];
                break;
            }
        }

        if (next.windowOpen) {
            for (int i = 0; i < 6; ++i) {
                if (prayerList[i].name != next.currentPrayer) continue;
                const int startSec = timeToMinutes(prayerList[i].time) * 60;
                int endSec = timeToMinutes(windowEnds[i]) * 60;
                int comparisonSec = nowSec;
                if (i == 5 && endSec < startSec) endSec += 86400;
                if (i == 5 && nowSec < startSec) comparisonSec += 86400;
                const int midpointSec = startSec + (endSec - startSec) / 2;
                if (comparisonSec < midpointSec) {
                    next.name = next.currentPrayer;
                    next.startTime = prayerList[i].time;
                }
                break;
            }
        }

        if (!foundUpcoming) {
            next.endTime = tomorrow.sunrise;
        } else {
            for (int i = 0; i < 6; ++i) {
                if (prayerList[i].name == next.name) {
                    next.endTime = windowEnds[i];
                    break;
                }
            }
        }

        if (foundUpcoming) {
            for (int i = 0; i < 6; ++i) {
                if (prayerList[i].name == next.upcomingPrayer) {
                    next.upcomingEndTime = windowEnds[i];
                    break;
                }
            }
        }
    }
}

bool isAzanTime(const DailyPrayers& prayers) {
    struct tm t;
    if (!getLocalTime(&t, 0) || t.tm_year < (2020 - 1900)) return false;

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
