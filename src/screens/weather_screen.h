#ifndef WEATHER_SCREEN_H
#define WEATHER_SCREEN_H

#include "../types.h"

// ============================================================
//  WEATHER SCREEN
//  Rotates through current conditions, AQI, pressure, and wind.
// ============================================================

namespace WeatherScreen {
    void draw(const WeatherData& weather, bool syncing,
              unsigned long timeOnScreenMs);
    void update(const WeatherData& weather, bool syncing,
                unsigned long timeOnScreenMs);
}

#endif // WEATHER_SCREEN_H
