#include "screens/weather_screen.h"
#include "display_manager.h"
#include "weather_manager.h"
#include "screens/weather_icons.h"
#include "config.h"

// ============================================================
//  WEATHER SCREEN (Screen 4)
//  City, temperature, humidity, AQI, weather icon.
// ============================================================

namespace WeatherScreen {

void draw(const WeatherData& weather) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    if (!weather.valid) {
        DisplayManager::showErrorScreen("Weather", "No data yet...");
        return;
    }

    // Header with city name
    oled.setFont(u8g2_font_6x10_tf);
    String header = weather.cityName + " WEATHER";
    if (header.length() > 21) header = header.substring(0, 21);
    oled.drawStr(2, 9, header.c_str());
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    // Weather icon on left
    WeatherIconType iconType = WeatherManager::getIconType(weather.weatherId);
    WeatherIcons::draw(oled, 8, 25, iconType);

    // Temperature
    oled.setFont(u8g2_font_logisoso24_tf);
    String tempText = String(weather.temperatureC, 1) + "C";
    oled.drawStr(49, 42, tempText.c_str());

    // Weather status
    oled.setFont(u8g2_font_6x10_tf);
    String status = weather.weatherMain;
    if (status.length() > 13) status = status.substring(0, 13);
    oled.drawStr(49, 53, status.c_str());

    // Bottom row: Humidity + AQI
    oled.setFont(u8g2_font_5x8_tf);
    String humStr = "H:" + String(weather.humidity) + "%";
    oled.drawStr(2, 63, humStr.c_str());

    String aqiStr = "AQI:" + String(weather.airQualityIndex) + " " +
                    WeatherManager::getAQIText(weather.airQualityIndex);
    if (aqiStr.length() > 16) aqiStr = aqiStr.substring(0, 16);
    oled.drawStr(50, 63, aqiStr.c_str());

    // Page indicator
    oled.drawStr(108, 63, "4/5");

    DisplayManager::sendBuffer();
}

} // namespace WeatherScreen
