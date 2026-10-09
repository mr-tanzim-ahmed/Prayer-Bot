#ifndef WEATHER_SCREEN_H
#define WEATHER_SCREEN_H

#include "types.h"

// ============================================================
//  WEATHER SCREEN (Screen 4)
//  Shows city name, temperature, humidity, AQI with category,
//  and the time of last update. Weather icon on left.
// ============================================================

namespace WeatherScreen {
    void draw(const WeatherData& weather);
}

#endif // WEATHER_SCREEN_H
