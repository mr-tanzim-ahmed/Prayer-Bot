#include "prayer_focus_screen.h"
#include "../display_manager.h"
#include "../prayer_manager.h"
#include "../config.h"
#include "screen_ui.h"

// ============================================================
//  PRAYER FOCUS SCREEN (Screen 1 - Default)
//  Next prayer, countdown, remaining window, prohibited time.
// ============================================================

namespace {
    String formatTime(uint8_t h, uint8_t m) {
        if (h == 0 && m == 0) return "00:00";
        const uint8_t hour12 = h % 12 == 0 ? 12 : h % 12;
        const char* meridiem = h < 12 ? "AM" : "PM";
        char buf[12];
        snprintf(buf, sizeof(buf), "%u:%02u %s", hour12, m, meridiem);
        return String(buf);
    }

    String formatCompactTime(uint8_t h, uint8_t m) {
        if (h == 0 && m == 0) return "00:00";
        const uint8_t hour12 = h % 12 == 0 ? 12 : h % 12;
        const char* meridiem = h < 12 ? "AM" : "PM";
        char buf[11];
        snprintf(buf, sizeof(buf), "%u:%02u%s", hour12, m, meridiem);
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
          bool azanPlaying, PrayerName activePrayer, bool syncing) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    if (azanPlaying) {
        ScreenUI::drawHeader(oled, "AZAN ALERT");
        ScreenUI::drawCard(oled, 3, 19, SCREEN_WIDTH - 6, 41);
        oled.setFont(u8g2_font_7x14B_tf);
        DisplayManager::drawCenteredText(
            String(PrayerManager::getPrayerNameStr(activePrayer)), 36);
        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText("Prayer alarm is sounding", 49);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Touch or shake to stop", 58);
        DisplayManager::sendBuffer();
        return;
    }

    ScreenUI::drawHeader(oled, "NEXT PRAYER", syncing ? "SYNC" : nullptr);
    ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 30);
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(8, 27, "UP NEXT");
    oled.setFont(u8g2_font_7x14B_tf);
    const char* name = PrayerManager::getPrayerNameStr(next.name);
    oled.drawStr(8, 43, name);

    String timeStr = formatTime(next.startTime.hour, next.startTime.minute);
    oled.setFont(u8g2_font_6x10_tf);
    const int tw = oled.getStrWidth(timeStr.c_str());
    oled.drawStr(SCREEN_WIDTH - tw - 8, 43, timeStr.c_str());

    oled.setFont(u8g2_font_5x8_tf);
    String countdown = "STARTS IN " + formatCountdown(next.countdownSeconds);
    DisplayManager::drawCenteredText(countdown, 55);

    if (prohibited.nextWindow.active) {
        oled.drawStr(8, 63, "PRAYER PAUSE ACTIVE");
    } else if (next.windowOpen) {
        String remaining = "PRAYER WINDOW " +
                           formatCountdown(next.remainingSeconds);
        oled.drawStr(8, 63, remaining.c_str());
    } else if (prohibited.nextWindow.startHour > 0 || prohibited.nextWindow.startMinute > 0) {
        String probStr = "PAUSE " +
            formatCompactTime(prohibited.nextWindow.startHour, prohibited.nextWindow.startMinute) +
            "-" +
            formatCompactTime(prohibited.nextWindow.endHour, prohibited.nextWindow.endMinute);
        oled.setFont(u8g2_font_4x6_tf);
        oled.drawStr(8, 63, probStr.c_str());
    }

    DisplayManager::sendBuffer();
}

void update(const NextPrayerInfo& next, const ProhibitedTimes& prohibited,
            bool azanPlaying, PrayerName activePrayer, bool syncing) {
    // Redraw every second for countdown updates
    draw(next, prohibited, azanPlaying, activePrayer, syncing);
}

} // namespace PrayerFocusScreen
