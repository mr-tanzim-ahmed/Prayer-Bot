#include "weather_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "cache_manager.h"

// ============================================================
//  WEATHER MANAGER IMPLEMENTATION
//  Fetches weather data from OpenWeatherMap API and air
//  quality data. Caches results for offline use.
// ============================================================

namespace WeatherManager {

bool update(const Settings& settings, WeatherData& weather) {
    Serial.println("[WEATHER] Updating weather data...");

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WEATHER] WiFi not connected.");
        return loadCache(weather);
    }

    bool weatherOK = false;
    bool aqiOK = false;

    if (settings.owmApiKey[0] == '\0') {
        Serial.println("[WEATHER] OpenWeather API key is not configured.");
    } else {
        // --- Fetch current weather from OpenWeatherMap ---
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        String url = String(OPENWEATHER_API_BASE) + "/weather"
            "?lat=" + String(settings.latitude, 4) +
            "&lon=" + String(settings.longitude, 4) +
            "&appid=" + String(settings.owmApiKey) +
            "&units=metric";

        if (http.begin(client, url)) {
            http.setTimeout(15000);
            int code = http.GET();

            if (code == HTTP_CODE_OK) {
                JsonDocument filter;
                filter["main"]["temp"] = true;
                filter["main"]["humidity"] = true;
                filter["weather"][0]["id"] = true;
                filter["weather"][0]["main"] = true;
                filter["weather"][0]["description"] = true;

                JsonDocument doc;
                if (!deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter))) {
                    weather.temperatureC      = doc["main"]["temp"] | 0.0f;
                    weather.humidity          = doc["main"]["humidity"] | 0;
                    weather.weatherId         = doc["weather"][0]["id"] | 0;
                    weather.weatherMain       = String((const char*)(doc["weather"][0]["main"] | "Unknown"));
                    weather.weatherDescription = String((const char*)(doc["weather"][0]["description"] | "Unknown"));
                    weather.cityName          = String(settings.cityName);
                    weatherOK = true;

                    Serial.printf("[WEATHER] Temp: %.1f C, Humidity: %d%%, Condition: %s\n",
                        weather.temperatureC, weather.humidity, weather.weatherMain.c_str());
                }
            } else {
                Serial.printf("[WEATHER] OpenWeather HTTP error: %d\n", code);
            }
            http.end();
        }
    }

    // --- Fetch air quality from Open-Meteo ---
    {
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        String url = "https://air-quality-api.open-meteo.com/v1/air-quality"
            "?latitude=" + String(settings.latitude, 4) +
            "&longitude=" + String(settings.longitude, 4) +
            "&current=pm2_5,european_aqi";

        if (http.begin(client, url)) {
            http.setTimeout(15000);
            int code = http.GET();

            if (code == HTTP_CODE_OK) {
                JsonDocument filter;
                filter["current"]["pm2_5"] = true;
                filter["current"]["european_aqi"] = true;

                JsonDocument doc;
                if (!deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter))) {
                    weather.airQualityIndex = doc["current"]["european_aqi"] | 0;
                    weather.pm25           = doc["current"]["pm2_5"] | 0.0f;
                    aqiOK = true;

                    Serial.printf("[WEATHER] AQI: %d, PM2.5: %.1f\n",
                        weather.airQualityIndex, weather.pm25);
                }
            }
            http.end();
        }
    }

    if (weatherOK || aqiOK) {
        weather.lastUpdateMs = millis();
        weather.valid = true;
        saveCache(weather);
        return true;
    }

    return loadCache(weather);
}

WeatherIconType getIconType(int weatherId) {
    if (weatherId >= 200 && weatherId < 300) return ICON_STORM;
    if (weatherId >= 300 && weatherId < 600) return ICON_RAIN;
    if (weatherId >= 600 && weatherId < 700) return ICON_SNOW;
    if (weatherId >= 700 && weatherId < 800) return ICON_FOG;
    if (weatherId == 800) return ICON_SUN;
    if (weatherId > 800 && weatherId < 900) return ICON_CLOUD;
    return ICON_SUN;
}

String getAQIText(int aqi) {
    // European AQI mapping (roughly)
    if (aqi <= 20) return "Good";
    if (aqi <= 40) return "Fair";
    if (aqi <= 60) return "Moderate";
    if (aqi <= 80) return "Poor";
    if (aqi > 80) return "Very Poor";
    return "No data";
}

bool loadCache(WeatherData& weather) {
    String json = StorageManager::readFile(WEATHER_CACHE_PATH);
    if (json.isEmpty()) return false;

    JsonDocument doc;
    if (deserializeJson(doc, json)) return false;

    weather.temperatureC       = doc["temp"] | 0.0f;
    weather.humidity           = doc["hum"]  | 0;
    weather.weatherId          = doc["wid"]  | 0;
    weather.weatherMain        = String((const char*)(doc["wm"] | "--"));
    weather.weatherDescription = String((const char*)(doc["wd"] | "--"));
    weather.airQualityIndex    = doc["aqi"]  | 0;
    weather.pm25               = doc["pm25"] | 0.0f;
    weather.cityName           = String((const char*)(doc["city"] | "--"));
    weather.valid              = true;

    Serial.println("[WEATHER] Loaded from cache.");
    return true;
}

bool saveCache(const WeatherData& weather) {
    JsonDocument doc;
    doc["temp"] = weather.temperatureC;
    doc["hum"]  = weather.humidity;
    doc["wid"]  = weather.weatherId;
    doc["wm"]   = weather.weatherMain;
    doc["wd"]   = weather.weatherDescription;
    doc["aqi"]  = weather.airQualityIndex;
    doc["pm25"] = weather.pm25;
    doc["city"] = weather.cityName;

    String json;
    serializeJson(doc, json);
    if (StorageManager::writeFile(WEATHER_CACHE_PATH, json)) {
        CacheManager::markUpdated(WEATHER_CACHE_PATH);
        return true;
    }
    return false;
}

} // namespace WeatherManager
