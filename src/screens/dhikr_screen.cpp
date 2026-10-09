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

void draw(unsigned long timeOnScreenMs) {
    U8G2& oled = DisplayManager::getDisplay();
    DisplayManager::clearBuffer();

    // 20 seconds per phrase, 5 phrases = 100 seconds total cycle
    unsigned long cycleTime = timeOnScreenMs % 100000;
    int index = cycleTime / 20000;
    if (index > 4) index = 4;

    // Remaining seconds for this specific dhikr (count down from 20)
    int remainingInPhrase = 20 - ((cycleTime % 20000) / 1000);

    // Phonetic Transliteration (Large)
    oled.setFont(u8g2_font_8x13_tf);
    DisplayManager::drawCenteredText(String(DHIKR_PHRASES[index]), 20);

    // Arabic Script (Fallback - depends on font support, will render but may be left-to-right unshaped on standard U8g2)
    oled.setFont(u8g2_font_unifont_t_arabic); // Unifont covers Arabic
    DisplayManager::drawCenteredText(String(DHIKR_ARABIC[index]), 40);

    // Progress bar for the 20 seconds
    int barWidth = map(remainingInPhrase, 0, 20, 0, SCREEN_WIDTH - 20);
    oled.drawFrame(10, 52, SCREEN_WIDTH - 20, 4);
    oled.drawBox(10, 52, barWidth, 4);

    // Page indicator
    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(108, 63, "6/6");

    DisplayManager::sendBuffer();
}

void update(unsigned long timeOnScreenMs) {
    // Redraw every second to animate progress bar
    draw(timeOnScreenMs);
}

} // namespace DhikrScreen
