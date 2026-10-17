#include "dhikr_screen.h"
#include "../display_manager.h"
#include "../config.h"
#include "../assets/dhikr/dhikr_arabic_art.h"

// ============================================================
//  DHIKR SCREEN IMPLEMENTATION
// ============================================================

namespace DhikrScreen {
namespace {
    constexpr unsigned long DHIKR_INTERVAL_MS = 5000UL;
    constexpr unsigned long DHIKR_CYCLE_MS = DHIKR_INTERVAL_MS * 5;
    uint32_t dhikrCount = 0;

    const uint8_t* const ARABIC_BITMAPS[] = {
        DhikrArabicArt::BITMAP_0,
        DhikrArabicArt::BITMAP_1,
        DhikrArabicArt::BITMAP_2,
        DhikrArabicArt::BITMAP_3,
        DhikrArabicArt::BITMAP_4
    };
    constexpr uint16_t ARABIC_WIDTHS[] = {
        DhikrArabicArt::WIDTH_0,
        DhikrArabicArt::WIDTH_1,
        DhikrArabicArt::WIDTH_2,
        DhikrArabicArt::WIDTH_3,
        DhikrArabicArt::WIDTH_4
    };

    void drawArabicPhrase(U8G2& oled, uint8_t index, unsigned long elapsedMs) {
        const uint16_t phraseWidth = ARABIC_WIDTHS[index];
        const int availableWidth = SCREEN_WIDTH - 8;
        constexpr int phraseY = 13;
        oled.setClipWindow(2, phraseY, SCREEN_WIDTH - 3,
                           phraseY + DhikrArabicArt::HEIGHT_0 - 1);
        if (phraseWidth <= availableWidth) {
            oled.drawXBM((SCREEN_WIDTH - phraseWidth) / 2, phraseY,
                         phraseWidth, DhikrArabicArt::HEIGHT_0, ARABIC_BITMAPS[index]);
        } else {
            const unsigned long travelDistance = SCREEN_WIDTH + phraseWidth + 4;
            const int x = SCREEN_WIDTH + 2 -
                (int)((elapsedMs * travelDistance) / DHIKR_INTERVAL_MS);
            oled.drawXBM(x, phraseY, phraseWidth, DhikrArabicArt::HEIGHT_0,
                         ARABIC_BITMAPS[index]);
        }
        oled.setMaxClipWindow();
    }
}

void draw(unsigned long timeOnScreenMs) {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    unsigned long cycleTime = timeOnScreenMs % DHIKR_CYCLE_MS;
    int index = cycleTime / DHIKR_INTERVAL_MS;
    int remainingSeconds = 5 - ((cycleTime % DHIKR_INTERVAL_MS) / 1000);

    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "ZIKIR");
    char phrasePosition[8];
    snprintf(phrasePosition, sizeof(phrasePosition), "%d/5", index + 1);
    oled.drawStr(104, 9, phrasePosition);
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    drawArabicPhrase(oled, index, cycleTime % DHIKR_INTERVAL_MS);

    char countText[24];
    snprintf(countText, sizeof(countText), "Count: %lu", (unsigned long)dhikrCount);
    oled.drawStr(2, 57, countText);
    char remainingText[16];
    snprintf(remainingText, sizeof(remainingText), "Next: %02ds", remainingSeconds);
    oled.drawStr(82, 57, remainingText);

    int elapsedMs = cycleTime % DHIKR_INTERVAL_MS;
    int barWidth = map(elapsedMs, 0, DHIKR_INTERVAL_MS, 0, SCREEN_WIDTH - 4);
    oled.drawFrame(2, 60, SCREEN_WIDTH - 4, 4);
    oled.drawBox(2, 60, barWidth, 4);

    DisplayManager::sendBuffer();
}

void update(unsigned long timeOnScreenMs) {
    draw(timeOnScreenMs);
}

void countTouch() {
    ++dhikrCount;
}

uint32_t getCount() {
    return dhikrCount;
}

} // namespace DhikrScreen
