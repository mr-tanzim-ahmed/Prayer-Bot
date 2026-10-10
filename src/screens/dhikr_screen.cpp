#include "screens/dhikr_screen.h"
#include "display_manager.h"
#include "config.h"

// ============================================================
//  DHIKR SCREEN IMPLEMENTATION
// ============================================================

const char* DHIKR_PHRASES[] = {
    "Subhanallah", 
    "Alhamdulillah", 
    "La ilaha illallah", 
    "Allahu Akbar", 
    "Astaghfirullah"
};

const char* DHIKR_ARABIC[] = {
    "سُبْحَانَ ٱللَّٰهِ", 
    "ٱلْحَمْدُ لِلَّٰهِ", 
    "لَا إِلٰهَ إِلَّا ٱللَّٰهُ", 
    "ٱللَّٰهُ أَكْبَرُ", 
    "أَسْتَغْفِرُ اللّٰهَ"
};

namespace DhikrScreen {
namespace {
    constexpr unsigned long DHIKR_INTERVAL_MS = 20000UL;
    constexpr unsigned long DHIKR_CYCLE_MS = 100000UL;
    uint32_t dhikrCount = 0;
}

void draw(unsigned long timeOnScreenMs) {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    unsigned long cycleTime = timeOnScreenMs % DHIKR_CYCLE_MS;
    int index = cycleTime / DHIKR_INTERVAL_MS;
    if (index > 4) index = 4;
    int remainingSeconds = 20 - ((cycleTime % DHIKR_INTERVAL_MS) / 1000);

    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "ZIKIR");
    char phrasePosition[8];
    snprintf(phrasePosition, sizeof(phrasePosition), "%d/5", index + 1);
    oled.drawStr(104, 9, phrasePosition);
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    oled.setFont(u8g2_font_unifont_t_arabic);
    DisplayManager::drawCenteredText(String(DHIKR_ARABIC[index]), 29);

    oled.setFont(u8g2_font_6x10_tf);
    DisplayManager::drawCenteredText(String(DHIKR_PHRASES[index]), 42);

    char countText[24];
    snprintf(countText, sizeof(countText), "Count: %lu", (unsigned long)dhikrCount);
    oled.drawStr(2, 53, countText);
    char remainingText[16];
    snprintf(remainingText, sizeof(remainingText), "Next: %02ds", remainingSeconds);
    oled.drawStr(82, 53, remainingText);

    int elapsedMs = cycleTime % DHIKR_INTERVAL_MS;
    int barWidth = map(elapsedMs, 0, DHIKR_INTERVAL_MS, 0, SCREEN_WIDTH - 20);
    oled.drawFrame(10, 57, SCREEN_WIDTH - 20, 5);
    oled.drawBox(10, 57, barWidth, 5);

    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(2, 63, "Touch either pad to count");

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
