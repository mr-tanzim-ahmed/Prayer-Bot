#include "surah_screen.h"
#include "../config.h"
#include "../display_manager.h"
#include "../voice_manager.h"
#include "screen_ui.h"

namespace SurahScreen {

namespace {
    void drawSurahTitle(uint8_t number, int16_t baseline) {
        char text[40];
        snprintf(text, sizeof(text), "%u. %s", number,
                 VoiceManager::getSurahName(number));
        U8G2& oled = DisplayManager::getDisplay();
        const int16_t width = oled.getStrWidth(text);
        if (width <= SCREEN_WIDTH - 8) {
            DisplayManager::drawCenteredText(text, baseline);
            return;
        }

        const int16_t gap = 24;
        const int16_t travel = width + gap;
        const int16_t offset = static_cast<int16_t>((millis() / 100) % travel);
        oled.drawStr(SCREEN_WIDTH - offset, baseline, text);
        oled.drawStr(SCREEN_WIDTH - offset + travel, baseline, text);
    }
}

void draw() {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    ScreenUI::drawHeader(oled, "QURAN PLAYER", "LISTEN");

    const unsigned long now = millis();

    if (VoiceManager::isListening()) {
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 32);
        const int cx = 64;
        const int cy = 31;
        const uint8_t pulse = static_cast<uint8_t>((now / 350) % 3);
        oled.drawCircle(cx, cy - 1, 12 + pulse);
        oled.drawRBox(cx - 3, cy - 9, 7, 11, 3);
        oled.drawCircle(cx, cy - 1, 7,
                        U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
        oled.drawVLine(cx, cy + 6, 4);
        oled.drawHLine(cx - 5, cy + 10, 11);

        const uint8_t frame = static_cast<uint8_t>((now / 260) % 6);
        const uint8_t heights[6][9] = {
            {3, 6, 10, 6, 12, 8, 4, 9, 3},
            {7, 11, 5, 9, 4, 12, 6, 10, 5},
            {4, 8, 12, 5, 10, 4, 11, 6, 9},
            {10, 4, 7, 12, 6, 9, 3, 11, 5},
            {5, 9, 3, 8, 12, 5, 10, 4, 7},
            {8, 5, 11, 4, 7, 12, 5, 9, 3}
        };
        for (int i = 0; i < 4; ++i) {
            const int height = heights[frame][i];
            oled.drawVLine(35 + i * 5, cy - height / 2, height);
            const int oppositeHeight = heights[frame][8 - i];
            oled.drawVLine(73 + i * 5, cy - oppositeHeight / 2,
                           oppositeHeight);
        }

        oled.setFont(u8g2_font_6x12_tf);
        DisplayManager::drawCenteredText("I'm listening...", 54);

        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("English / Bangla", 63);

    } else if (VoiceManager::isProcessing()) {
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 34);
        const uint8_t dotCount = (now / 350) % 4;
        char procText[32];
        snprintf(procText, sizeof(procText), "Recognizing%.*s", dotCount, "...");

        oled.setFont(u8g2_font_6x12_tf);
        DisplayManager::drawCenteredText(procText, 33);

        const int barW = 28 + ((now / 120) % 68);
        oled.drawRFrame(15, 39, 98, 7, 3);
        oled.drawRBox(16, 40, barW, 5, 2);

        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Gemini is thinking", 63);

    } else if (VoiceManager::isPlaying()) {
        const uint8_t surah = VoiceManager::getPlayingSurah();
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 37);
        oled.setFont(u8g2_font_helvB10_tf);
        drawSurahTitle(surah, 32);

        const uint8_t frame = (now / 260) % 6;
        const uint8_t eqHeights[6][11] = {
            {3, 6, 9, 13, 8, 4, 11, 7, 12, 6, 4},
            {5, 11, 4, 8, 14, 7, 5, 12, 8, 10, 5},
            {8, 5, 12, 6, 9, 13, 7, 4, 11, 5, 8},
            {12, 9, 6, 11, 5, 8, 14, 9, 6, 12, 4},
            {6, 13, 8, 4, 11, 6, 8, 13, 5, 7, 11},
            {4, 8, 11, 7, 12, 5, 10, 6, 9, 13, 6}
        };

        const int startX = 29;
        for (int b = 0; b < 11; ++b) {
            const int h = eqHeights[frame][b];
            oled.drawVLine(startX + b * 6, 47 - h, h);
            oled.drawVLine(startX + b * 6 + 1, 47 - h, h);
        }

        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Now playing", 52);
        DisplayManager::drawCenteredText("[Tap] Next   [Shake] Stop", 63);

    } else {
        const uint8_t surah = VoiceManager::getPlayingSurah();
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 37);
        oled.setFont(u8g2_font_helvB10_tf);
        drawSurahTitle(surah, 34);

        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText(VoiceManager::getStatusText(), 49);

        oled.setFont(u8g2_font_5x8_tf);
        if (VoiceManager::hasError())
            DisplayManager::drawCenteredText("Check Wi-Fi, mic, Gemini", 63);
        else
            DisplayManager::drawCenteredText("[Tap] Next   [Shake] Play", 63);
    }

    DisplayManager::sendBuffer();
}

} // namespace SurahScreen
