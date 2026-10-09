#include "screens/clock_calendar_screen.h"
#include "display_manager.h"
#include "screens/weather_icons.h"
#include "config.h"
#include <time.h>

// ============================================================
//  CLOCK & CALENDAR SCREEN (Screen 3)
//  Current time, Gregorian date, Hijri date, sunrise, sunset.
// ============================================================

namespace ClockCalendarScreen {

void draw(const HijriDate& hijri, const IslamicEvent& event, const DailyPrayers& prayers) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    struct tm t;
    bool timeValid = getLocalTime(&t);

    // Current time (large)
    oled.setFont(u8g2_font_logisoso24_tf);
    if (timeValid) {
        char timeBuf[6];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", t.tm_hour, t.tm_min);
        DisplayManager::drawCenteredText(String(timeBuf), 28);
    } else {
        DisplayManager::drawCenteredText("--:--", 28);
    }

    // Gregorian date
    oled.setFont(u8g2_font_5x8_tf);
    if (timeValid) {
        char dateBuf[16];
        snprintf(dateBuf, sizeof(dateBuf), "%02d/%02d/%04d", t.tm_mday, t.tm_mon + 1, t.tm_year + 1900);
        DisplayManager::drawCenteredText(String(dateBuf), 38);
    }

    // Hijri date
    char hijriBuf[40];
    snprintf(hijriBuf, sizeof(hijriBuf), "%d %s %d AH", hijri.day, hijri.monthName.c_str(), hijri.year);
    DisplayManager::drawCenteredText(String(hijriBuf), 48);

    oled.drawHLine(0, 52, SCREEN_WIDTH);
    
    // Festival banner (if active, overrides sunrise/sunset to save space, or we can just flash them)
    if (!event.name.isEmpty() && (t.tm_sec % 4 < 2)) {
        // Flash festival banner every 2 seconds
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText(event.name, 62);
    } else {
        // Sunrise (Left)
        // Manual mini sun icon
        oled.drawCircle(8, 58, 3);
        oled.drawLine(8, 53, 8, 54);
        oled.drawLine(8, 62, 8, 63);
        oled.drawLine(3, 58, 4, 58);
        oled.drawLine(12, 58, 13, 58);
        
        char srBuf[8];
        snprintf(srBuf, sizeof(srBuf), "%02d:%02d", prayers.sunrise.hour, prayers.sunrise.minute);
        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(15, 62, srBuf);
        
        // Sunset / Maghrib (Right)
        // Manual half sun (sunset) icon
        oled.drawCircle(SCREEN_WIDTH - 40, 61, 3, U8G2_DRAW_UPPER_RIGHT | U8G2_DRAW_UPPER_LEFT);
        oled.drawHLine(SCREEN_WIDTH - 44, 61, 9);
        
        char ssBuf[8];
        snprintf(ssBuf, sizeof(ssBuf), "%02d:%02d", prayers.maghrib.hour, prayers.maghrib.minute);
        oled.drawStr(SCREEN_WIDTH - 30, 62, ssBuf);
    }

    // Page indicator
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(108, 63, "3/5");

    DisplayManager::sendBuffer();
}

void update(const HijriDate& hijri, const IslamicEvent& event, const DailyPrayers& prayers) {
    // Redraw every second for clock and flashing banner
    draw(hijri, event, prayers);
}

} // namespace ClockCalendarScreen
