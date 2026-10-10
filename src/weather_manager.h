#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include "types.h"

// ============================================================
//  WEATHER MANAGER
//  Fetches current weather and air quality data from APIs.
//  Manages caching for offline use.
// ============================================================

namespace WeatherManager {
    // Fetch weather + AQI and update the WeatherData struct
    bool update(const Settings& settings, WeatherData& weather);

    // Get weather icon type from OpenWeather condition ID
    WeatherIconType getIconType(int weatherId);

    // Get AQI category text
    String getAQIText(int aqi);

    // Cache management
    bool loadCache(WeatherData& weather);
    bool saveCache(const WeatherData& weather);
}

#endif // WEATHER_MANAGER_H
