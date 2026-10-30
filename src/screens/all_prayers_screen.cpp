#include "all_prayers_screen.h"
#include "../display_manager.h"
#include "../prayer_manager.h"
#include "../config.h"
#include "screen_ui.h"

// ============================================================
//  ALL PRAYERS SCREEN (Screen 2)
//  Shows the five daily prayers with aligned, easy-to-read times.
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
}

namespace AllPrayersScreen {

void draw(const DailyPrayers& prayers, const NextPrayerInfo& next) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    ScreenUI::drawHeader(oled, "TODAY'S PRAYERS", "SALAH");

    struct {
        const char* name;
        PrayerTime time;
        PrayerName prayerName;
    } rows[] = {
        {"Fajr",    prayers.fajr,    PRAYER_FAJR},
        {"Dhuhr",   prayers.dhuhr,   PRAYER_DHUHR},
        {"Asr",     prayers.asr,     PRAYER_ASR},
        {"Maghrib", prayers.maghrib, PRAYER_MAGHRIB},
        {"Isha",    prayers.isha,    PRAYER_ISHA}
    };

    oled.setFont(u8g2_font_5x8_tf);
    for (int i = 0; i < 5; i++) {
        const int y = 17 + i * 9;
        const bool isNext = rows[i].prayerName == next.name;
        if (isNext) {
            ScreenUI::drawPill(oled, 3, y, SCREEN_WIDTH - 6, 9);
            oled.setDrawColor(0);
        }

        oled.drawStr(9, y + 7, rows[i].name);
        oled.drawStr(54, y + 7, "->");
        String t = formatTime(rows[i].time.hour, rows[i].time.minute);
        const int timeWidth = oled.getStrWidth(t.c_str());
        oled.drawStr(SCREEN_WIDTH - 9 - timeWidth, y + 7, t.c_str());

        oled.setDrawColor(1);
    }

    DisplayManager::sendBuffer();
}

void drawSahariIftar(const DailyPrayers& prayers) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();
    ScreenUI::drawHeader(oled, "SAHARI & IFTAR", "TODAY");

    struct {
        const char* label;
        PrayerTime time;
    } rows[] = {
        {"SAHARI ENDS", prayers.fajr},
        {"IFTAR", prayers.maghrib}
    };

    for (uint8_t i = 0; i < 2; ++i) {
        const int y = 19 + i * 22;
        ScreenUI::drawCard(oled, 4, y, SCREEN_WIDTH - 8, 20);
        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(10, y + 8, rows[i].label);

        String time = formatTime(rows[i].time.hour, rows[i].time.minute);
        oled.setFont(u8g2_font_7x14B_tf);
        const int timeWidth = oled.getStrWidth(time.c_str());
        oled.drawStr(SCREEN_WIDTH - timeWidth - 10, y + 18, time.c_str());
    }

    DisplayManager::sendBuffer();
}

} // namespace AllPrayersScreen
