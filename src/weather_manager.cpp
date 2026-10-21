#include "weather_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "cache_manager.h"
#include <math.h>

// ============================================================
//  WEATHER MANAGER IMPLEMENTATION
//  Fetches weather and air quality data from OpenWeatherMap and caches it.
// ============================================================

namespace {

bool isJsonNumber(JsonVariantConst value) {
    return value.is<int>() || value.is<float>();
}

bool cacheMatchesLocation(const WeatherData& weather,
                          const Settings& settings) {
    return isfinite(weather.latitude) && isfinite(weather.longitude) &&
        fabsf(weather.latitude - settings.latitude) <= 0.0001f &&
        fabsf(weather.longitude - settings.longitude) <= 0.0001f;
}

bool readPollutant(JsonVariantConst value, float& result) {
    if (!isJsonNumber(value)) return false;
    const float amount = value.as<float>();
    if (!isfinite(amount) || amount < 0.0f || amount > 1000000.0f)
        return false;
    result = amount;
    return true;
}

}

namespace WeatherManager {

bool update(const Settings& settings, WeatherData& weather) {
    Serial.println("[WEATHER] Updating weather data...");

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WEATHER] WiFi not connected.");
        WeatherData cached = {};
        if (loadCache(cached) && cacheMatchesLocation(cached, settings)) {
            weather = cached;
            weather.cityName = String(settings.cityName);
            return true;
        }
        weather = {};
        weather.cityName = String(settings.cityName);
        return false;
    }

    WeatherData cached = {};
    if (loadCache(cached) && cacheMatchesLocation(cached, settings)) {
        weather = cached;
    } else {
        weather = {};
    }
    weather.latitude = settings.latitude;
    weather.longitude = settings.longitude;
    weather.cityName = String(settings.cityName);

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
                filter["main"]["pressure"] = true;
                filter["weather"][0]["id"] = true;
                filter["weather"][0]["main"] = true;
                filter["weather"][0]["description"] = true;
                filter["weather"][0]["icon"] = true;
                filter["wind"]["speed"] = true;

                JsonDocument doc;
                if (!deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter))) {
                    JsonVariantConst temperature = doc["main"]["temp"];
                    JsonVariantConst humidity = doc["main"]["humidity"];
                    JsonVariantConst conditionId = doc["weather"][0]["id"];
                    if (isJsonNumber(temperature) && humidity.is<int>() &&
                        conditionId.is<int>()) {
                        const float temperatureC = temperature.as<float>();
                        const int humidityPercent = humidity.as<int>();
                        const int weatherId = conditionId.as<int>();
                        if (isfinite(temperatureC) && temperatureC >= -100.0f &&
                            temperatureC <= 70.0f && humidityPercent >= 0 &&
                            humidityPercent <= 100 && weatherId > 0 &&
                            weatherId < 1000) {
                            weather.temperatureC = temperatureC;
                            weather.humidity = humidityPercent;
                            const int pressureHpa =
                                doc["main"]["pressure"] | 0;
                            weather.pressureHpa =
                                pressureHpa > 0 && pressureHpa <= 1200
                                    ? pressureHpa : 0;
                            const JsonVariantConst windSpeed =
                                doc["wind"]["speed"];
                            weather.windValid = false;
                            weather.windSpeedKmh = 0.0f;
                            if (isJsonNumber(windSpeed)) {
                                const float speedKmh =
                                    windSpeed.as<float>() * 3.6f;
                                if (isfinite(speedKmh) && speedKmh >= 0.0f &&
                                    speedKmh <= 540.0f) {
                                    weather.windSpeedKmh = speedKmh;
                                    weather.windValid = true;
                                }
                            }
                            weather.weatherId = weatherId;
                            weather.weatherMain = String(
                                (const char*)(doc["weather"][0]["main"] | "Unknown"));
                            weather.weatherDescription = String(
                                (const char*)(doc["weather"][0]["description"] | "Unknown"));
                            weather.iconCode = String(
                                (const char*)(doc["weather"][0]["icon"] | ""));
                            weather.cityName = String(settings.cityName);
                            weather.valid = true;
                            weatherOK = true;

                            Serial.printf("[WEATHER] Temp: %.1f C, Humidity: %d%%, Condition: %s\n",
                                weather.temperatureC, weather.humidity, weather.weatherMain.c_str());
                        } else {
                            Serial.println("[WEATHER] OpenWeather returned out-of-range current weather data.");
                        }
                    } else {
                        Serial.println("[WEATHER] OpenWeather current-weather response is missing required fields.");
                    }
                } else {
                    Serial.println("[WEATHER] Could not parse OpenWeather current-weather response.");
                }
            } else {
                Serial.printf("[WEATHER] OpenWeather HTTP error: %d\n", code);
            }
            http.end();
        }
    }

    if (settings.owmApiKey[0] == '\0') {
        return weather.valid || weather.airQualityValid;
    }

    // --- Fetch air quality and pollutant concentrations from OpenWeatherMap ---
    {
        WiFiClientSecure client;
        client.setInsecure();

        HTTPClient http;
        String url = String(OPENWEATHER_API_BASE) + "/air_pollution"
            "?lat=" + String(settings.latitude, 4) +
            "&lon=" + String(settings.longitude, 4) +
            "&appid=" + String(settings.owmApiKey);

        if (http.begin(client, url)) {
            http.setTimeout(15000);
            int code = http.GET();

            if (code == HTTP_CODE_OK) {
                JsonDocument filter;
                filter["list"][0]["main"]["aqi"] = true;
                filter["list"][0]["components"]["pm2_5"] = true;
                filter["list"][0]["components"]["pm10"] = true;
                filter["list"][0]["components"]["co"] = true;
                filter["list"][0]["components"]["no"] = true;
                filter["list"][0]["components"]["no2"] = true;
                filter["list"][0]["components"]["o3"] = true;
                filter["list"][0]["components"]["so2"] = true;
                filter["list"][0]["components"]["nh3"] = true;

                JsonDocument doc;
                if (!deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter))) {
                    JsonObjectConst current = doc["list"][0];
                    if (!current.isNull() &&
                        current["main"]["aqi"].is<int>()) {
                        JsonObjectConst components = current["components"];
                        const int aqi = current["main"]["aqi"] | 0;
                        float pm25;
                        float pm10;
                        float carbonMonoxide;
                        float nitrogenMonoxide;
                        float nitrogenDioxide;
                        float ozone;
                        float sulphurDioxide;
                        float ammonia;
                        const bool componentsValid =
                            readPollutant(components["pm2_5"], pm25) &&
                            readPollutant(components["pm10"], pm10) &&
                            readPollutant(components["co"], carbonMonoxide) &&
                            readPollutant(components["no"], nitrogenMonoxide) &&
                            readPollutant(components["no2"], nitrogenDioxide) &&
                            readPollutant(components["o3"], ozone) &&
                            readPollutant(components["so2"], sulphurDioxide) &&
                            readPollutant(components["nh3"], ammonia);
                        if (aqi >= 1 && aqi <= 5 && componentsValid) {
                            weather.airQualityIndex = aqi;
                            weather.pm25 = pm25;
                            weather.pm10 = pm10;
                            weather.carbonMonoxide = carbonMonoxide;
                            weather.nitrogenMonoxide = nitrogenMonoxide;
                            weather.nitrogenDioxide = nitrogenDioxide;
                            weather.ozone = ozone;
                            weather.sulphurDioxide = sulphurDioxide;
                            weather.ammonia = ammonia;
                            weather.airQualityValid = true;
                            aqiOK = true;
                            Serial.printf("[WEATHER] OpenWeather AQI: %d, PM2.5: %.1f, PM10: %.1f\n",
                                aqi, pm25, pm10);
                        } else {
                            Serial.println("[WEATHER] OpenWeather AQI or pollutant values are outside supported limits.");
                        }
                    } else {
                        Serial.println("[WEATHER] OpenWeather air-quality response has no current reading.");
                    }
                }
            } else {
                Serial.printf("[WEATHER] OpenWeather air-quality HTTP error: %d\n", code);
            }
            http.end();
        }
    }

    if (weatherOK || aqiOK) {
        weather.lastUpdateMs = millis();
        saveCache(weather);
        return true;
    }

    return weather.valid || weather.airQualityValid;
}

WeatherIconType getIconType(int weatherId) {
    if (weatherId >= 200 && weatherId < 300) return ICON_STORM;
    if (weatherId >= 300 && weatherId < 600) return ICON_RAIN;
    if (weatherId >= 600 && weatherId < 700) return ICON_SNOW;
    if (weatherId >= 700 && weatherId < 800) {
        if (weatherId == 771 || weatherId == 781) return ICON_STORM;
        return ICON_FOG;
    }
    if (weatherId == 800) return ICON_SUN;
    if (weatherId == 801 || weatherId == 802) return ICON_PARTLY_CLOUDY;
    if (weatherId > 800 && weatherId < 900) return ICON_CLOUD;
    return ICON_CLOUD;
}

WeatherIconType getIconType(int weatherId, const String& iconCode) {
    if (iconCode.length() == 3 &&
        (iconCode[2] == 'd' || iconCode[2] == 'n')) {
        const bool isNight = iconCode[2] == 'n';
        const String conditionCode = iconCode.substring(0, 2);
        if (conditionCode == "01") return isNight ? ICON_MOON : ICON_SUN;
        if (conditionCode == "02")
            return isNight ? ICON_NIGHT_CLOUDY : ICON_PARTLY_CLOUDY;
        if (conditionCode == "03" || conditionCode == "04")
            return ICON_CLOUD;
        if (conditionCode == "09" || conditionCode == "10")
            return ICON_RAIN;
        if (conditionCode == "11") return ICON_STORM;
        if (conditionCode == "13") return ICON_SNOW;
        if (conditionCode == "50") return ICON_FOG;
    }

    if (weatherId == 800 && iconCode.endsWith("n")) return ICON_MOON;
    return getIconType(weatherId);
}

String getAQIText(int aqi) {
    switch (aqi) {
        case 1: return "Good";
        case 2: return "Fair";
        case 3: return "Moderate";
        case 4: return "Poor";
        case 5: return "Very Poor";
        default: return "No data";
    }
}

bool loadCache(WeatherData& weather) {
    String json = StorageManager::readFile(WEATHER_CACHE_PATH);
    if (json.isEmpty()) return false;

    JsonDocument doc;
    if (deserializeJson(doc, json)) return false;

    JsonVariantConst temperature = doc["temp"];
    JsonVariantConst humidity = doc["hum"];
    JsonVariantConst conditionId = doc["wid"];
    if (!isJsonNumber(temperature) || !humidity.is<int>() ||
        !conditionId.is<int>()) {
        Serial.println("[WEATHER] Cached weather data is missing required fields.");
        return false;
    }

    WeatherData cached = {};
    cached.temperatureC = temperature.as<float>();
    cached.humidity = humidity.as<int>();
    cached.pressureHpa = doc["pressureHpa"] | 0;
    cached.windSpeedKmh = doc["windKmh"] | 0.0f;
    JsonVariantConst latitude = doc["lat"];
    JsonVariantConst longitude = doc["lon"];
    if (!isJsonNumber(latitude) || !isJsonNumber(longitude)) {
        Serial.println("[WEATHER] Cached weather data has no location; discarding it.");
        return false;
    }
    cached.latitude = latitude.as<float>();
    cached.longitude = longitude.as<float>();
    cached.windValid = doc["windValid"] | false;
    cached.weatherId = conditionId.as<int>();
    if (!isfinite(cached.temperatureC) || cached.temperatureC < -100.0f ||
        cached.temperatureC > 70.0f || cached.humidity < 0 ||
        cached.humidity > 100 || cached.weatherId < 0 ||
        cached.weatherId >= 1000 || cached.pressureHpa < 0 ||
        cached.pressureHpa > 1200 || !isfinite(cached.latitude) ||
        cached.latitude < -90.0f || cached.latitude > 90.0f ||
        !isfinite(cached.longitude) || cached.longitude < -180.0f ||
        cached.longitude > 180.0f || !isfinite(cached.windSpeedKmh) ||
        cached.windSpeedKmh < 0.0f || cached.windSpeedKmh > 540.0f) {
        Serial.println("[WEATHER] Cached weather data is outside supported limits.");
        return false;
    }
    cached.weatherMain = String((const char*)(doc["wm"] | "--"));
    cached.weatherDescription = String((const char*)(doc["wd"] | "--"));
    cached.iconCode = String((const char*)(doc["icon"] | ""));
    cached.airQualityIndex = doc["aqi"] | 0;
    const bool pollutantValuesValid =
        readPollutant(doc["pm25"], cached.pm25) &&
        readPollutant(doc["pm10"], cached.pm10) &&
        readPollutant(doc["co"], cached.carbonMonoxide) &&
        readPollutant(doc["no"], cached.nitrogenMonoxide) &&
        readPollutant(doc["no2"], cached.nitrogenDioxide) &&
        readPollutant(doc["o3"], cached.ozone) &&
        readPollutant(doc["so2"], cached.sulphurDioxide) &&
        readPollutant(doc["nh3"], cached.ammonia);
    cached.cityName = String((const char*)(doc["city"] | "--"));
    cached.valid = (doc["weatherValid"] | (cached.weatherId > 0)) &&
        cached.weatherId > 0;
    cached.airQualityValid = doc["aqiValid"] | false;
    cached.airQualityValid = cached.airQualityValid &&
        cached.airQualityIndex >= 1 && cached.airQualityIndex <= 5 &&
        pollutantValuesValid;
    if (cached.airQualityIndex < 1 || cached.airQualityIndex > 5)
        cached.airQualityIndex = 0;
    if (!cached.airQualityValid) cached.airQualityIndex = 0;

    weather = cached;

    Serial.println("[WEATHER] Loaded from cache.");
    return cached.valid || cached.airQualityValid;
}

bool saveCache(const WeatherData& weather) {
    JsonDocument doc;
    doc["temp"] = weather.temperatureC;
    doc["hum"]  = weather.humidity;
    doc["pressureHpa"] = weather.pressureHpa;
    doc["windKmh"] = weather.windSpeedKmh;
    doc["lat"] = weather.latitude;
    doc["lon"] = weather.longitude;
    doc["windValid"] = weather.windValid;
    doc["wid"]  = weather.weatherId;
    doc["wm"]   = weather.weatherMain;
    doc["wd"]   = weather.weatherDescription;
    doc["icon"] = weather.iconCode;
    doc["weatherValid"] = weather.valid;
    doc["aqi"]  = weather.airQualityIndex;
    doc["aqiValid"] = weather.airQualityValid;
    doc["pm25"] = weather.pm25;
    doc["pm10"] = weather.pm10;
    doc["co"] = weather.carbonMonoxide;
    doc["no"] = weather.nitrogenMonoxide;
    doc["no2"] = weather.nitrogenDioxide;
    doc["o3"] = weather.ozone;
    doc["so2"] = weather.sulphurDioxide;
    doc["nh3"] = weather.ammonia;
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
