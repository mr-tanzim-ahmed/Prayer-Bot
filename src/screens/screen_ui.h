#ifndef SCREEN_UI_H
#define SCREEN_UI_H

#include <U8g2lib.h>
#include "../config.h"

namespace ScreenUI {

inline void drawHeader(U8G2& oled, const char* title, const char* tag = nullptr) {
    oled.setDrawColor(1);
    oled.drawRBox(0, 0, SCREEN_WIDTH, 15, 4);
    oled.setDrawColor(0);
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(6, 11, title);
    if (tag) {
        oled.setFont(u8g2_font_5x8_tf);
        const int width = oled.getStrWidth(tag);
        oled.drawStr(SCREEN_WIDTH - width - 6, 10, tag);
    }
    oled.setDrawColor(1);
}

inline void drawCard(U8G2& oled, int x, int y, int width, int height) {
    oled.drawRFrame(x, y, width, height, 5);
}

inline void drawPill(U8G2& oled, int x, int y, int width, int height) {
    oled.drawRBox(x, y, width, height, height / 2);
}

inline void drawProgress(U8G2& oled, int x, int y, int width,
                         int progress, int maximum) {
    oled.drawRFrame(x, y, width, 5, 2);
    if (maximum > 0 && progress > 0) {
        const int fill = ((width - 2) * progress) / maximum;
        if (fill > 0) oled.drawRBox(x + 1, y + 1, fill, 3, 1);
    }
}

} // namespace ScreenUI

#endif // SCREEN_UI_H
