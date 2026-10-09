#include "screens/clock_calendar_screen.h"
#include "display_manager.h"
#include "config.h"
#include <time.h>

// ============================================================
//  CLOCK & CALENDAR SCREEN (Screen 3)
//  Current time, Gregorian date, Hijri date, festival banner.
// ============================================================

namespace ClockCalendarScreen {

void draw(const HijriDate& hijri, const IslamicEvent& event) {
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

    // Festival banner
    if (!event.name.isEmpty()) {
        oled.drawHLine(0, 51, SCREEN_WIDTH);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText(event.name, 61);
    }

    // Page indicator
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(108, 63, "3/5");

    DisplayManager::sendBuffer();
}

void update(const HijriDate& hijri, const IslamicEvent& event) {
    // Redraw every second for clock
    draw(hijri, event);
}

} // namespace ClockCalendarScreen
