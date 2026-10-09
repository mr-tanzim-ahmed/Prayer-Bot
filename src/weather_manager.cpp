#include "weather_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

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

    // --- Fetch current weather from Open-Meteo ---
    {
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        String url = "https://api.open-meteo.com/v1/forecast"
            "?latitude=" + String(settings.latitude, 4) +
            "&longitude=" + String(settings.longitude, 4) +
            "&current=temperature_2m,relative_humidity_2m,weather_code";

        if (http.begin(client, url)) {
            http.setTimeout(15000);
            int code = http.GET();

            if (code == HTTP_CODE_OK) {
                // Setup ArduinoJson filter to save memory
                JsonDocument filter;
                filter["current"]["temperature_2m"] = true;
                filter["current"]["relative_humidity_2m"] = true;
                filter["current"]["weather_code"] = true;

                JsonDocument doc;
                if (!deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter))) {
                    weather.temperatureC      = doc["current"]["temperature_2m"] | 0.0f;
                    weather.humidity          = doc["current"]["relative_humidity_2m"] | 0;
                    weather.weatherId         = doc["current"]["weather_code"] | 0;
                    weather.cityName          = String(settings.cityName);
                    weatherOK = true;

                    Serial.printf("[WEATHER] Temp: %.1f°C, Humidity: %d%%, Code: %d\n",
                        weather.temperatureC, weather.humidity, weather.weatherId);
                }
            } else {
                Serial.printf("[WEATHER] HTTP error: %d\n", code);
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
        // Derive text from WMO code
        weather.weatherMain = getStatusText(String(weather.weatherId));
        weather.weatherDescription = weather.weatherMain;
        
        weather.lastUpdateMs = millis();
        weather.valid = true;
        saveCache(weather);
        return true;
    }

    return loadCache(weather);
}

WeatherIconType getIconType(int weatherCode) {
    // WMO Weather interpretation codes
    if (weatherCode == 0) return ICON_SUN; // Clear
    if (weatherCode >= 1 && weatherCode <= 3) return ICON_CLOUD; // Partly cloudy
    if (weatherCode >= 45 && weatherCode <= 48) return ICON_FOG; // Fog
    if (weatherCode >= 51 && weatherCode <= 67) return ICON_RAIN; // Drizzle/Rain
    if (weatherCode >= 71 && weatherCode <= 77) return ICON_SNOW; // Snow
    if (weatherCode >= 80 && weatherCode <= 82) return ICON_RAIN; // Showers
    if (weatherCode >= 85 && weatherCode <= 86) return ICON_SNOW; // Snow showers
    if (weatherCode >= 95 && weatherCode <= 99) return ICON_STORM; // Thunderstorm
    return ICON_SUN;
}

String getStatusText(const String& codeStr) {
    int code = codeStr.toInt();
    if (code == 0) return "Clear";
    if (code == 1 || code == 2) return "Partly Cloudy";
    if (code == 3) return "Overcast";
    if (code == 45 || code == 48) return "Fog";
    if (code >= 51 && code <= 57) return "Drizzle";
    if (code >= 61 && code <= 67) return "Rain";
    if (code >= 71 && code <= 77) return "Snow";
    if (code >= 80 && code <= 82) return "Showers";
    if (code >= 85 && code <= 86) return "Snow Showers";
    if (code >= 95 && code <= 99) return "Thunderstorm";
    return "Unknown";
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

#include "cache_manager.h"

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
