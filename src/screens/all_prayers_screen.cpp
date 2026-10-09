#include "screens/all_prayers_screen.h"
#include "display_manager.h"
#include "prayer_manager.h"
#include "config.h"

// ============================================================
//  ALL PRAYERS SCREEN (Screen 2)
//  Shows all 5 prayer times + sunrise.
// ============================================================

namespace {
    String formatTime(uint8_t h, uint8_t m) {
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", h, m);
        return String(buf);
    }
}

namespace AllPrayersScreen {

void draw(const DailyPrayers& prayers, const NextPrayerInfo& next) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    // Header
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "ALL PRAYERS");
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    oled.setFont(u8g2_font_5x8_tf);

    struct {
        const char* name;
        PrayerTime time;
        PrayerName prayerName;
    } rows[] = {
        {"Fajr",    prayers.fajr,    PRAYER_FAJR},
        {"Sunrise", prayers.sunrise,  PRAYER_SUNRISE},
        {"Dhuhr",   prayers.dhuhr,   PRAYER_DHUHR},
        {"Asr",     prayers.asr,     PRAYER_ASR},
        {"Maghrib", prayers.maghrib, PRAYER_MAGHRIB},
        {"Isha",    prayers.isha,    PRAYER_ISHA}
    };

    int y = 22;
    for (int i = 0; i < 6; i++) {
        // Highlight next prayer
        if (rows[i].prayerName == next.name) {
            oled.drawBox(0, y - 7, SCREEN_WIDTH, 9);
            oled.setDrawColor(0);
        }

        oled.drawStr(4, y, rows[i].name);
        String t = formatTime(rows[i].time.hour, rows[i].time.minute);
        int tw = oled.getUTF8Width(t.c_str());
        oled.drawStr(SCREEN_WIDTH - tw - 4, y, t.c_str());

        oled.setDrawColor(1);
        y += 8;
    }

    // Page indicator
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(108, 63, "2/5");

    DisplayManager::sendBuffer();
}

} // namespace AllPrayersScreen
