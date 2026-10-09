#ifndef WEATHER_ICONS_H
#define WEATHER_ICONS_H

#include <U8g2lib.h>
#include "types.h"

// ============================================================
//  WEATHER ICONS
//  Pixel-art weather icons for OLED display.
// ============================================================

namespace WeatherIcons {

inline void drawSun(U8G2& oled, int x, int y) {
    oled.drawCircle(x + 17, y + 15, 9);
    oled.drawLine(x + 17, y + 1,  x + 17, y + 5);
    oled.drawLine(x + 17, y + 25, x + 17, y + 29);
    oled.drawLine(x + 3,  y + 15, x + 7,  y + 15);
    oled.drawLine(x + 27, y + 15, x + 31, y + 15);
    oled.drawLine(x + 7,  y + 5,  x + 10, y + 8);
    oled.drawLine(x + 24, y + 22, x + 27, y + 25);
    oled.drawLine(x + 27, y + 5,  x + 24, y + 8);
    oled.drawLine(x + 10, y + 22, x + 7,  y + 25);
}

inline void drawCloud(U8G2& oled, int x, int y) {
    oled.drawCircle(x + 12, y + 17, 7);
    oled.drawCircle(x + 22, y + 14, 9);
    oled.drawCircle(x + 31, y + 18, 6);
    oled.drawBox(x + 10, y + 17, 25, 9);
}

inline void drawRain(U8G2& oled, int x, int y) {
    drawCloud(oled, x, y - 3);
    oled.drawLine(x + 14, y + 29, x + 11, y + 35);
    oled.drawLine(x + 23, y + 29, x + 20, y + 35);
    oled.drawLine(x + 32, y + 29, x + 29, y + 35);
}

inline void drawStorm(U8G2& oled, int x, int y) {
    drawCloud(oled, x, y - 3);
    oled.drawLine(x + 22, y + 27, x + 16, y + 37);
    oled.drawLine(x + 16, y + 37, x + 22, y + 37);
    oled.drawLine(x + 22, y + 37, x + 18, y + 45);
}

inline void drawSnow(U8G2& oled, int x, int y) {
    drawCloud(oled, x, y - 3);
    oled.drawLine(x + 14, y + 30, x + 14, y + 36);
    oled.drawLine(x + 11, y + 33, x + 17, y + 33);
    oled.drawLine(x + 25, y + 30, x + 25, y + 36);
    oled.drawLine(x + 22, y + 33, x + 28, y + 33);
    oled.drawLine(x + 34, y + 30, x + 34, y + 36);
    oled.drawLine(x + 31, y + 33, x + 37, y + 33);
}

inline void drawFog(U8G2& oled, int x, int y) {
    oled.drawLine(x + 5, y + 12, x + 35, y + 12);
    oled.drawLine(x + 2, y + 20, x + 38, y + 20);
    oled.drawLine(x + 7, y + 28, x + 33, y + 28);
}

inline void draw(U8G2& oled, int x, int y, WeatherIconType type) {
    switch (type) {
        case ICON_SUN:   drawSun(oled, x, y);   break;
        case ICON_CLOUD: drawCloud(oled, x, y); break;
        case ICON_RAIN:  drawRain(oled, x, y);  break;
        case ICON_STORM: drawStorm(oled, x, y); break;
        case ICON_SNOW:  drawSnow(oled, x, y);  break;
        case ICON_FOG:   drawFog(oled, x, y);   break;
        default:         drawSun(oled, x, y);   break;
    }
}

} // namespace WeatherIcons

#endif // WEATHER_ICONS_H
