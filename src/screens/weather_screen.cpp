#include "weather_screen.h"
#include "../display_manager.h"
#include "../weather_manager.h"
#include "weather_icons.h"
#include "../config.h"
#include "screen_ui.h"

namespace {
    constexpr unsigned long CURRENT_PAGE_MS = 3000UL;
    constexpr unsigned long AIR_QUALITY_PAGE_MS = 2000UL;
    constexpr unsigned long DETAILS_PAGE_MS = 3000UL;
    constexpr unsigned long WEATHER_CYCLE_MS =
        CURRENT_PAGE_MS + AIR_QUALITY_PAGE_MS + DETAILS_PAGE_MS;

    enum class WeatherPage : uint8_t {
        CURRENT,
        AIR_QUALITY,
        DETAILS
    };

    WeatherPage getPage(unsigned long timeOnScreenMs) {
        const unsigned long cyclePosition = timeOnScreenMs % WEATHER_CYCLE_MS;
        if (cyclePosition < CURRENT_PAGE_MS) return WeatherPage::CURRENT;
        if (cyclePosition < CURRENT_PAGE_MS + AIR_QUALITY_PAGE_MS)
            return WeatherPage::AIR_QUALITY;
        return WeatherPage::DETAILS;
    }

    String clippedText(U8G2& oled, String text, int maxWidth) {
        while (oled.getStrWidth(text.c_str()) > maxWidth &&
               text.length() > 3) {
            text.remove(text.length() - 1, 1);
        }
        return text;
    }

    void drawCenteredAt(U8G2& oled, const String& text, int centerX, int y) {
        oled.drawStr(centerX - oled.getStrWidth(text.c_str()) / 2, y,
                     text.c_str());
    }

    void drawEmpty(U8G2& oled, bool syncing) {
        ScreenUI::drawHeader(oled, "WEATHER", syncing ? "SYNC" : nullptr);
        ScreenUI::drawCard(oled, 3, 19, SCREEN_WIDTH - 6, 40);
        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText(syncing ? "Finding forecast..." :
                                                   "No forecast yet", 36);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText(
            syncing ? "Please wait" : "Check Wi-Fi and location", 48);
    }

    void drawCurrent(U8G2& oled, const WeatherData& weather, bool syncing) {
        ScreenUI::drawHeader(oled, "WEATHER",
                             syncing ? "SYNCING" : "OPENWEATHER");

        String city = clippedText(oled, weather.cityName, SCREEN_WIDTH - 12);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText(city, 25);

        if (weather.valid) {
            const WeatherIconType iconType =
                WeatherManager::getIconType(weather.weatherId,
                                            weather.iconCode);
            WeatherIcons::drawCompact(oled, 9, 32, iconType);
        }

        oled.setFont(u8g2_font_logisoso20_tf);
        String temperature = weather.valid
            ? String(weather.temperatureC, 1) + "C"
            : "--";
        oled.drawStr(40, 49, temperature.c_str());

        oled.setFont(u8g2_font_5x8_tf);
        String condition = weather.valid
            ? clippedText(oled, weather.weatherMain, 54)
            : "Waiting for data";
        oled.drawStr(40, 62, condition.c_str());
        const String humidity = weather.valid
            ? String(weather.humidity) + "%"
            : "--";
        oled.drawStr(118 - oled.getStrWidth(humidity.c_str()), 62,
                     humidity.c_str());
    }

    void drawAirQuality(U8G2& oled, const WeatherData& weather, bool syncing) {
        ScreenUI::drawHeader(oled, "AIR QUALITY",
                             syncing ? "SYNCING" : "OWM 1-5");
        ScreenUI::drawCard(oled, 4, 19, SCREEN_WIDTH - 8, 43);

        if (!weather.airQualityValid) {
            oled.setFont(u8g2_font_5x8_tf);
            DisplayManager::drawCenteredText(
                syncing ? "Updating AQI..." : "AQI unavailable", 39);
            DisplayManager::drawCenteredText("Check network or API access", 52);
            return;
        }

        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(11, 30, "AQI INDEX");

        char aqiText[4];
        snprintf(aqiText, sizeof(aqiText), "%d", weather.airQualityIndex);
        oled.setFont(u8g2_font_logisoso20_tf);
        oled.drawStr(12, 56, aqiText);

        String category = WeatherManager::getAQIText(weather.airQualityIndex);
        oled.setFont(u8g2_font_6x10_tf);
        oled.drawStr(43, 44, category.c_str());

        oled.setFont(u8g2_font_5x8_tf);
        String pm25 = "PM2.5 " + String(weather.pm25, 1) + " ug/m3";
        drawCenteredAt(oled, pm25, 84, 57);
    }

    void drawDetails(U8G2& oled, const WeatherData& weather, bool syncing) {
        ScreenUI::drawHeader(oled, "DETAILS",
                             syncing ? "SYNCING" : "OWM");

        oled.setFont(u8g2_font_5x8_tf);
        String now = weather.valid
            ? "NOW  " + String(weather.temperatureC, 1) + "C"
            : "CURRENT CONDITIONS";
        now = clippedText(oled, now, SCREEN_WIDTH - 12);
        DisplayManager::drawCenteredText(now, 26);

        ScreenUI::drawCard(oled, 4, 31, 58, 31);
        ScreenUI::drawCard(oled, 66, 31, 58, 31);
        oled.setFont(u8g2_font_5x8_tf);
        drawCenteredAt(oled, "PRESSURE", 33, 42);
        drawCenteredAt(oled, "WIND", 95, 42);

        oled.setFont(u8g2_font_5x8_tf);
        const String pressure = weather.pressureHpa > 0
            ? String(weather.pressureHpa) + " hPa"
            : "--";
        const String wind = weather.windValid
            ? String(weather.windSpeedKmh, 1) + " km/h"
            : "--";
        drawCenteredAt(oled, pressure, 33, 56);
        drawCenteredAt(oled, wind, 95, 56);
    }
}

namespace WeatherScreen {

void draw(const WeatherData& weather, bool syncing,
          unsigned long timeOnScreenMs) {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    if (!weather.valid && !weather.airQualityValid) {
        drawEmpty(oled, syncing);
        DisplayManager::sendBuffer();
        return;
    }

    switch (getPage(timeOnScreenMs)) {
        case WeatherPage::CURRENT:
            drawCurrent(oled, weather, syncing);
            break;
        case WeatherPage::AIR_QUALITY:
            drawAirQuality(oled, weather, syncing);
            break;
        case WeatherPage::DETAILS:
            drawDetails(oled, weather, syncing);
            break;
    }

    DisplayManager::sendBuffer();
}

void update(const WeatherData& weather, bool syncing,
            unsigned long timeOnScreenMs) {
    draw(weather, syncing, timeOnScreenMs);
}

} // namespace WeatherScreen
