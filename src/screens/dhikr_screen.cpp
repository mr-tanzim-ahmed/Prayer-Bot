#include "dhikr_screen.h"
#include "../display_manager.h"
#include "../config.h"

namespace DhikrScreen {
namespace {
    constexpr unsigned long DHIKR_INTERVAL_MS = 5000UL;
    constexpr unsigned long DHIKR_CYCLE_MS = DHIKR_INTERVAL_MS * 5;
    uint32_t dhikrCount = 0;

    struct DhikrPhrase {
        const char* transliteration;
        const char* meaningLine1;
        const char* meaningLine2;
    };

    const DhikrPhrase PHRASES[] = {
        {"Subhan Allah", "Glory be to Allah", ""},
        {"Alhamdulillah", "Praise be to Allah", ""},
        {"La ilaha illallah", "There is no deity", "except Allah"},
        {"Allahu Akbar", "Allah is the Greatest", ""},
        {"Astaghfirullah", "I seek Allah's", "forgiveness"}
    };
}

void draw(unsigned long timeOnScreenMs) {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    const unsigned long cycleTime = timeOnScreenMs % DHIKR_CYCLE_MS;
    const uint8_t index = cycleTime / DHIKR_INTERVAL_MS;
    const int remainingSeconds =
        5 - ((cycleTime % DHIKR_INTERVAL_MS) / 1000);
    const DhikrPhrase& phrase = PHRASES[index];

    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "DHIKR");
    char phrasePosition[8];
    snprintf(phrasePosition, sizeof(phrasePosition), "%u/5",
             static_cast<unsigned>(index + 1));
    oled.drawStr(78, 9, phrasePosition);
    oled.drawStr(108, 9, "6/7");
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    oled.setFont(u8g2_font_helvB12_tf);
    DisplayManager::drawCenteredText(phrase.transliteration, 29);

    oled.setFont(u8g2_font_5x8_tf);
    DisplayManager::drawCenteredText(phrase.meaningLine1, 43);
    if (phrase.meaningLine2[0] != '\0') {
        DisplayManager::drawCenteredText(phrase.meaningLine2, 52);
    }

    const int elapsedMs = cycleTime % DHIKR_INTERVAL_MS;
    const int barWidth = map(elapsedMs, 0, DHIKR_INTERVAL_MS,
                             0, SCREEN_WIDTH - 4);
    oled.drawFrame(2, 56, SCREEN_WIDTH - 4, 3);
    if (barWidth > 0) oled.drawBox(3, 57, barWidth - 1, 1);

    char countText[24];
    snprintf(countText, sizeof(countText), "Count: %lu",
             static_cast<unsigned long>(dhikrCount));
    oled.drawStr(2, 63, countText);
    char remainingText[12];
    snprintf(remainingText, sizeof(remainingText), "Next:%02ds",
             remainingSeconds);
    oled.drawStr(91, 63, remainingText);

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
