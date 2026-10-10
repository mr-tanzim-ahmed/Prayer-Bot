#include "screens/prayer_focus_screen.h"
#include "display_manager.h"
#include "prayer_manager.h"
#include "config.h"

// ============================================================
//  PRAYER FOCUS SCREEN (Screen 1 - Default)
//  Next prayer, countdown, remaining window, prohibited time.
// ============================================================

namespace {
    String formatTime(uint8_t h, uint8_t m) {
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
        return String(buf);
    }

    String formatCountdown(int32_t totalSeconds) {
        if (totalSeconds <= 0) return "00:00:00";
        int h = totalSeconds / 3600;
        int m = (totalSeconds % 3600) / 60;
        int s = totalSeconds % 60;
        char buf[9];
        snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
        return String(buf);
    }
}

namespace PrayerFocusScreen {

void draw(const NextPrayerInfo& next, const ProhibitedTimes& prohibited,
          bool azanPlaying, PrayerName activePrayer) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    if (azanPlaying) {
        oled.setFont(u8g2_font_6x10_tf);
        oled.drawStr(2, 9, "PRAYER TIME");
        oled.drawHLine(0, 12, SCREEN_WIDTH);

        oled.setFont(u8g2_font_7x14B_tf);
        DisplayManager::drawCenteredText(
            String(PrayerManager::getPrayerNameStr(activePrayer)), 32);

        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText("Beep alarm active", 45);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Touch or shake to stop", 59);
        DisplayManager::sendBuffer();
        return;
    }

    // Header
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "NEXT PRAYER");
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    // Prayer name and time
    oled.setFont(u8g2_font_7x14B_tf);
    const char* name = PrayerManager::getPrayerNameStr(next.name);
    oled.drawStr(2, 26, name);

    String timeStr = formatTime(next.startTime.hour, next.startTime.minute);
    int tw = oled.getUTF8Width(timeStr.c_str());
    oled.drawStr(SCREEN_WIDTH - tw - 2, 26, timeStr.c_str());

    // Countdown
    oled.setFont(u8g2_font_6x10_tf);
    String countdown = "In: " + formatCountdown(next.countdownSeconds);
    oled.drawStr(2, 38, countdown.c_str());

    // Time left to pray (if window is open)
    if (next.windowOpen) {
        String remaining = "Left: " + formatCountdown(next.remainingSeconds);
        oled.drawStr(2, 48, remaining.c_str());
    }

    // Next prohibited time
    oled.setFont(u8g2_font_5x8_tf);
    if (prohibited.nextWindow.active) {
        oled.drawStr(2, 58, "! PROHIBITED NOW");
    } else if (prohibited.nextWindow.startHour > 0 || prohibited.nextWindow.startMinute > 0) {
        String probStr = "Makruh: " +
            formatTime(prohibited.nextWindow.startHour, prohibited.nextWindow.startMinute) +
            "-" +
            formatTime(prohibited.nextWindow.endHour, prohibited.nextWindow.endMinute);
        oled.drawStr(2, 58, probStr.c_str());
    }

    // Page indicator
    oled.drawStr(108, 63, "1/5");

    DisplayManager::sendBuffer();
}

void update(const NextPrayerInfo& next, const ProhibitedTimes& prohibited,
            bool azanPlaying, PrayerName activePrayer) {
    // Redraw every second for countdown updates
    draw(next, prohibited, azanPlaying, activePrayer);
}

} // namespace PrayerFocusScreen
