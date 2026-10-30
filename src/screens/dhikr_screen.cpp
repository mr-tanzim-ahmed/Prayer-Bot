#include "dhikr_screen.h"
#include "../display_manager.h"
#include "../config.h"
#include "screen_ui.h"

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

    char phrasePosition[8];
    snprintf(phrasePosition, sizeof(phrasePosition), "%u/5",
             static_cast<unsigned>(index + 1));
    ScreenUI::drawHeader(oled, "DHIKR", phrasePosition);

    ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 29);
    oled.setFont(u8g2_font_helvB10_tf);
    DisplayManager::drawCenteredText(phrase.transliteration, 33);

    oled.setFont(u8g2_font_5x8_tf);
    DisplayManager::drawCenteredText(phrase.meaningLine1, 41);
    if (phrase.meaningLine2[0] != '\0') {
        DisplayManager::drawCenteredText(phrase.meaningLine2, 48);
    }

    const int elapsedMs = cycleTime % DHIKR_INTERVAL_MS;
    ScreenUI::drawProgress(oled, 4, 52, SCREEN_WIDTH - 8,
                           elapsedMs, DHIKR_INTERVAL_MS);

    char countText[24];
    snprintf(countText, sizeof(countText), "Count: %lu",
             static_cast<unsigned long>(dhikrCount));
    oled.drawStr(4, 63, countText);
    char remainingText[12];
    snprintf(remainingText, sizeof(remainingText), "Next:%02ds",
             remainingSeconds);
    oled.drawStr(93, 63, remainingText);

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
