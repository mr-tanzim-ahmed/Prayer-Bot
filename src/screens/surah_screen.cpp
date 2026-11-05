#include "screens/surah_screen.h"
#include <prayer_bot_config.h>
#include "managers/device/display_manager.h"
#include "managers/audio/voice_manager.h"
#include "screens/screen_ui.h"

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

    void drawPlaybackResult(uint8_t number) {
        U8G2& oled = DisplayManager::getDisplay();
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 37);

        constexpr int centerX = SCREEN_WIDTH / 2;
        constexpr int centerY = 30;
        oled.drawCircle(centerX, centerY, 8);
        oled.drawTriangle(centerX - 2, centerY - 4,
                          centerX - 2, centerY + 4,
                          centerX + 4, centerY);

        char title[40];
        snprintf(title, sizeof(title), "%u. %s", number,
                 VoiceManager::getSurahName(number));
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText(title, 52);
    }
}

void draw() {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    ScreenUI::drawHeader(oled, "QURAN PLAYER", "LISTEN");

    const unsigned long now = millis();

    if (VoiceManager::isListening()) {
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 45);
        const int cx = 64;
        const int cy = 34;
        const uint8_t pulse = static_cast<uint8_t>((now / 280) % 3);
        oled.drawCircle(cx, cy, 12 + pulse);
        oled.drawRBox(cx - 3, cy - 8, 7, 10, 3);
        oled.drawCircle(cx, cy, 7,
                        U8G2_DRAW_LOWER_LEFT | U8G2_DRAW_LOWER_RIGHT);
        oled.drawVLine(cx, cy + 7, 4);
        oled.drawHLine(cx - 5, cy + 11, 11);

        oled.setFont(u8g2_font_6x12_tf);
        DisplayManager::drawCenteredText("Listening...", 54);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Speak in English / Bangla", 62);

    } else if (VoiceManager::isProcessing()) {
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 45);
        const int cx = 64;
        const int cy = 33;
        const uint8_t scanFrame = static_cast<uint8_t>((now / 140) % 4);
        oled.drawCircle(cx - 3, cy - 3, 9);
        oled.drawLine(cx + 3, cy + 3, cx + 11, cy + 11);
        static const int8_t scanX[4] = {0, 9, 0, -9};
        static const int8_t scanY[4] = {-9, 0, 9, 0};
        for (uint8_t i = 0; i < 4; ++i) {
            const int x = cx - 3 + scanX[i];
            const int y = cy - 3 + scanY[i];
            oled.drawDisc(x, y, i == scanFrame ? 2 : 1);
        }
        oled.drawDisc(cx - 3, cy - 3, 2);
        oled.setFont(u8g2_font_6x12_tf);
        DisplayManager::drawCenteredText("Recognizing Surah...", 55);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Finding Quran chapter", 63);

    } else if (VoiceManager::isPlaying()) {
        const uint8_t surah = VoiceManager::getPlayingSurah();
        drawPlaybackResult(surah);

        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("PLAYING", 63);

    } else if (VoiceManager::wasLastSelectionVoiceRecognized() &&
               !VoiceManager::hasError()) {
        drawPlaybackResult(VoiceManager::getRecognizedSurah());
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("SURAH FOUND", 63);

    } else {
        const uint8_t surah = VoiceManager::getPlayingSurah();
        ScreenUI::drawCard(oled, 3, 18, SCREEN_WIDTH - 6, 37);
        oled.setFont(u8g2_font_helvB10_tf);
        if (surah == 0)
            DisplayManager::drawCenteredText("Select a Surah", 34);
        else
            drawSurahTitle(surah, 34);

        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText(VoiceManager::getStatusText(), 49);

        oled.setFont(u8g2_font_5x8_tf);
        if (VoiceManager::hasError())
            DisplayManager::drawCenteredText("Tap to choose; shake to play", 63);
        else
            DisplayManager::drawCenteredText("[Tap] Next   [Shake] Play", 63);
    }

    DisplayManager::sendBuffer();
}

} // namespace SurahScreen
