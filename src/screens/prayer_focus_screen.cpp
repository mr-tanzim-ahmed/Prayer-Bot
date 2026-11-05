#include "screens/prayer_focus_screen.h"
#include "managers/device/display_manager.h"
#include "managers/prayer/prayer_manager.h"
#include <prayer_bot_config.h>
#include "screens/screen_ui.h"

// ============================================================
//  PRAYER FOCUS SCREEN (Screen 1 - Default)
//  Current and next prayer pages, countdown, and prohibited time.
// ============================================================

namespace {
    bool validPrayerTime(const PrayerTime& time) {
        return time.hour <= 23 && time.minute <= 59;
    }

    String formatCompactTime(uint8_t h, uint8_t m) {
        const PrayerTime time{h, m};
        if (!validPrayerTime(time)) return "--:--";
        const uint8_t hour12 = h % 12 == 0 ? 12 : h % 12;
        const char* meridiem = h < 12 ? "AM" : "PM";
        char buf[11];
        snprintf(buf, sizeof(buf), "%u:%02u%s", hour12, m, meridiem);
        return String(buf);
    }

    PrayerTime getPrayerStart(const DailyPrayers& prayers,
                              PrayerName prayer) {
        switch (prayer) {
            case PRAYER_FAJR: return prayers.fajr;
            case PRAYER_DHUHR: return prayers.dhuhr;
            case PRAYER_ASR: return prayers.asr;
            case PRAYER_MAGHRIB: return prayers.maghrib;
            case PRAYER_ISHA: return prayers.isha;
            default: return {UINT8_MAX, UINT8_MAX};
        }
    }

    PrayerTime getPrayerEnd(const DailyPrayers& prayers,
                            PrayerName prayer) {
        switch (prayer) {
            case PRAYER_FAJR: return prayers.sunrise;
            case PRAYER_DHUHR: return prayers.asr;
            case PRAYER_ASR: return prayers.maghrib;
            case PRAYER_MAGHRIB: return prayers.isha;
            case PRAYER_ISHA: return prayers.fajr;
            default: return {UINT8_MAX, UINT8_MAX};
        }
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

void draw(const DailyPrayers& prayers, const NextPrayerInfo& next,
          const ProhibitedTimes& prohibited, bool azanPlaying,
          PrayerName activePrayer, bool syncing, bool showNextPrayer) {
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

    const PrayerName displayPrayer = showNextPrayer
        ? next.upcomingPrayer
        : (next.windowOpen ? next.currentPrayer : next.presentPrayer);
    const bool hasDisplayPrayer = displayPrayer < PRAYER_NONE;
    const bool previousIshaOvernight = !showNextPrayer &&
        !next.windowOpen && displayPrayer == PRAYER_ISHA &&
        next.upcomingPrayer == PRAYER_FAJR;
    const PrayerTime displayStart = showNextPrayer
        ? next.upcomingStartTime
        : (next.windowOpen ? next.currentStartTime
                           : (previousIshaOvernight
                                  ? PrayerTime{UINT8_MAX, UINT8_MAX}
                                  : getPrayerStart(prayers, displayPrayer)));
    const PrayerTime displayEnd = showNextPrayer
        ? next.upcomingEndTime
        : (next.windowOpen ? next.windowEndTime
                           : (previousIshaOvernight
                                  ? next.upcomingStartTime
                                  : getPrayerEnd(prayers, displayPrayer)));
    const char* title = showNextPrayer ? "NEXT PRAYER"
        : (next.windowOpen ? "CURRENT PRAYER" : "LAST PRAYER");
    ScreenUI::drawHeader(oled, title, syncing ? "SYNC" : nullptr);
    ScreenUI::drawCard(oled, 3, 16, SCREEN_WIDTH - 6, 34);
    oled.setFont(u8g2_font_7x14B_tf);
    oled.drawStr(8, 30, hasDisplayPrayer
        ? PrayerManager::getPrayerNameStr(displayPrayer)
        : "Unavailable");

    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(8, 40, "START");
    oled.drawStr(34, 40, formatCompactTime(
        displayStart.hour, displayStart.minute).c_str());
    oled.drawStr(69, 40, "END");
    oled.drawStr(88, 40, formatCompactTime(
        displayEnd.hour, displayEnd.minute).c_str());

    String countdown;
    if (showNextPrayer) {
        countdown = "STARTS IN " + formatCountdown(next.countdownSeconds);
    } else if (next.windowOpen) {
        countdown = "ENDS IN " + formatCountdown(next.remainingSeconds);
    } else {
        countdown = hasDisplayPrayer ? "WINDOW CLOSED" : "Prayer data unavailable";
    }
    DisplayManager::drawCenteredText(countdown, 56);

    oled.setFont(u8g2_font_4x6_tf);
    if (prohibited.nextWindow.active) {
        oled.drawStr(8, 63, "PRAYER PAUSE ACTIVE");
    } else if (prohibited.nextWindow.startHour > 0 || prohibited.nextWindow.startMinute > 0) {
        String probStr = "PAUSE " +
            formatCompactTime(prohibited.nextWindow.startHour, prohibited.nextWindow.startMinute) +
            "-" +
            formatCompactTime(prohibited.nextWindow.endHour, prohibited.nextWindow.endMinute);
        oled.drawStr(8, 63, probStr.c_str());
    }

    DisplayManager::sendBuffer();
}

void update(const DailyPrayers& prayers, const NextPrayerInfo& next,
            const ProhibitedTimes& prohibited, bool azanPlaying,
            PrayerName activePrayer, bool syncing, bool showNextPrayer) {
    // Redraw every second for countdown updates
    draw(prayers, next, prohibited, azanPlaying, activePrayer, syncing,
         showNextPrayer);
}

} // namespace PrayerFocusScreen
