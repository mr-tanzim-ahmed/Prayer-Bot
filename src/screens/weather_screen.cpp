#include "weather_screen.h"
#include "../display_manager.h"
#include "../weather_manager.h"
#include "weather_icons.h"
#include "../config.h"

// ============================================================
//  WEATHER SCREEN (Screen 4)
//  City, temperature, humidity, AQI, weather icon.
// ============================================================

namespace WeatherScreen {

namespace {
    void drawSyncIndicator(U8G2& oled, bool syncing) {
        const int cx = SCREEN_WIDTH - 7;
        const int cy = 7;
        const uint8_t frame = syncing
            ? static_cast<uint8_t>((millis() / 140UL) % 4UL)
            : 0;
        static const int8_t directions[4][2] = {
            {0, -1}, {1, 0}, {0, 1}, {-1, 0}
        };

        for (uint8_t i = 0; i < 4; ++i) {
            const uint8_t direction = (i + frame) % 4;
            const int8_t dx = directions[direction][0];
            const int8_t dy = directions[direction][1];
            const int length = syncing && i == 0 ? 4 : 2;
            oled.drawLine(cx, cy, cx + dx * length, cy + dy * length);
        }
        if (!syncing) oled.drawCircle(cx, cy, 4);
    }
}

void draw(const WeatherData& weather, bool syncing) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    if (!weather.valid) {
        oled.setFont(u8g2_font_6x10_tf);
        oled.drawStr(2, 9, "WEATHER");
        drawSyncIndicator(oled, syncing);
        oled.drawHLine(0, 12, SCREEN_WIDTH);
        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText(
            syncing ? "Syncing weather..." : "No weather data", 32);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText(
            syncing ? "Please wait" : "Check Wi-Fi and location", 47);
        oled.drawStr(108, 63, "4/7");
        DisplayManager::sendBuffer();
        return;
    }

    oled.setFont(u8g2_font_6x10_tf);
    String header = weather.cityName + " WEATHER";
    while (oled.getStrWidth(header.c_str()) > 103 && header.length() > 8) {
        header.remove(header.length() - 8, 8);
    }
    oled.drawStr(2, 9, header.c_str());
    drawSyncIndicator(oled, syncing);
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    WeatherIconType iconType = WeatherManager::getIconType(weather.weatherId);
    WeatherIcons::draw(oled, 8, 25, iconType);

    oled.setFont(u8g2_font_logisoso24_tf);
    String tempText = String(weather.temperatureC, 1) + "C";
    oled.drawStr(49, 42, tempText.c_str());

    oled.setFont(u8g2_font_6x10_tf);
    String status = weather.weatherMain;
    if (status.length() > 13) status = status.substring(0, 13);
    oled.drawStr(49, 53, status.c_str());

    oled.setFont(u8g2_font_5x8_tf);
    String humStr = "H:" + String(weather.humidity) + "%";
    oled.drawStr(2, 63, humStr.c_str());

    String aqiStr = "AQI:" + String(weather.airQualityIndex) + " " +
                    WeatherManager::getAQIText(weather.airQualityIndex);
    if (aqiStr.length() > 16) aqiStr = aqiStr.substring(0, 16);
    oled.drawStr(50, 63, aqiStr.c_str());

    oled.drawStr(108, 63, "4/7");

    DisplayManager::sendBuffer();
}

void update(const WeatherData& weather, bool syncing) {
    draw(weather, syncing);
}

} // namespace WeatherScreen
