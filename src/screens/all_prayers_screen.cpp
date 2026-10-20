#include "all_prayers_screen.h"
#include "../display_manager.h"
#include "../prayer_manager.h"
#include "../config.h"
#include "screen_ui.h"

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

    ScreenUI::drawHeader(oled, "TODAY'S PRAYERS", "SALAH");

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

    for (int i = 0; i < 6; i++) {
        const int column = i % 2;
        const int row = i / 2;
        const int x = column == 0 ? 2 : 65;
        const int y = 18 + row * 15;
        const bool isNext = rows[i].prayerName == next.name;
        if (isNext) {
            ScreenUI::drawPill(oled, x, y, 61, 14);
            oled.setDrawColor(0);
        } else {
            ScreenUI::drawCard(oled, x, y, 61, 14);
        }

        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(x + 4, y + 10, rows[i].name);
        String t = formatTime(rows[i].time.hour, rows[i].time.minute);
        oled.setFont(u8g2_font_6x10_tf);
        const int tw = oled.getStrWidth(t.c_str());
        oled.drawStr(x + 61 - tw - 4, y + 10, t.c_str());

        oled.setDrawColor(1);
    }

    DisplayManager::sendBuffer();
}

} // namespace AllPrayersScreen
