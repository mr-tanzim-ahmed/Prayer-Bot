#include "clock_calendar_screen.h"
#include "../display_manager.h"
#include "../config.h"
#include "screen_ui.h"
#include <time.h>

// ============================================================
//  CLOCK & CALENDAR SCREEN (Screen 3)
//  Current time, Gregorian date, Hijri date, sunrise, sunset.
// ============================================================

namespace ClockCalendarScreen {

void draw(const HijriDate& hijri, const IslamicEvent& event, const DailyPrayers& prayers) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();
    ScreenUI::drawHeader(oled, "CLOCK & DATE", "LOCAL");

    struct tm t;
    bool timeValid = getLocalTime(&t);

    ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 24);
    if (timeValid) {
        const int hour12 = (t.tm_hour % 12 == 0) ? 12 : t.tm_hour % 12;
        const char* meridiem = t.tm_hour < 12 ? "AM" : "PM";
        char timeBuf[8];
        snprintf(timeBuf, sizeof(timeBuf), "%02d:%02d", hour12, t.tm_min);
        oled.setFont(u8g2_font_logisoso18_tf);
        const int timeWidth = oled.getStrWidth(timeBuf);
        const int startX = (SCREEN_WIDTH - timeWidth - 20) / 2;
        oled.drawStr(startX, 37, timeBuf);
        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(startX + timeWidth + 5, 36, meridiem);
    } else {
        oled.setFont(u8g2_font_logisoso18_tf);
        DisplayManager::drawCenteredText("--:--", 28);
    }

    oled.setFont(u8g2_font_5x8_tf);
    if (timeValid) {
        static const char* const monthNames[] = {
            "Jan", "Feb", "Mar", "Apr", "May", "Jun",
            "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
        };
        char dateBuf[24];
        snprintf(dateBuf, sizeof(dateBuf), "Local: %02d %s %04d",
                 t.tm_mday, monthNames[t.tm_mon], t.tm_year + 1900);
        DisplayManager::drawCenteredText(String(dateBuf), 49);
    }

    char hijriBuf[40];
    snprintf(hijriBuf, sizeof(hijriBuf), "%d %s %d AH", hijri.day,
             hijri.monthName.c_str(), hijri.year);
    oled.setFont(u8g2_font_5x8_tf);
    DisplayManager::drawCenteredText(String(hijriBuf), 56);

    if (!event.name.isEmpty()) {
        oled.setFont(u8g2_font_5x8_tf);
        String eventText = event.isWeekly ? "JUMUAH · " + event.name : event.name;
        while (oled.getStrWidth(eventText.c_str()) > SCREEN_WIDTH - 20 &&
               eventText.length() > 4) {
            eventText.remove(eventText.length() - 1, 1);
        }
        DisplayManager::drawCenteredText(eventText, 63);
    } else {
        oled.setFont(u8g2_font_5x8_tf);
        char sunTimes[24];
        snprintf(sunTimes, sizeof(sunTimes), "RISE %02u:%02u   SET %02u:%02u",
                 prayers.sunrise.hour, prayers.sunrise.minute,
                 prayers.maghrib.hour, prayers.maghrib.minute);
        DisplayManager::drawCenteredText(String(sunTimes), 63);
    }

    DisplayManager::sendBuffer();
}

void update(const HijriDate& hijri, const IslamicEvent& event, const DailyPrayers& prayers) {
    // Redraw every second for the clock.
    draw(hijri, event, prayers);
}

} // namespace ClockCalendarScreen
