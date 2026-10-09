#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "config.h"

// ============================================================
//  DISPLAY MANAGER
//  Handles OLED initialization, health monitoring, recovery,
//  and provides drawing utilities used by all screens.
// ============================================================

namespace DisplayManager {
    // Initialization
    bool init();
    bool isAvailable();

    // Health monitoring - call periodically
    void service();

    // Get the U8G2 instance for direct drawing
    U8G2& getDisplay();

    // Drawing utilities
    void clearBuffer();
    void sendBuffer();
    void drawCenteredText(const String& text, int y);
    void drawText(const String& text, int x, int y);
    void drawHLine(int x, int y, int w);

    // Common screens
    void showStartupScreen();
    void showConnectingScreen();
    void showErrorScreen(const String& line1, const String& line2);
    void showListeningScreen();
    void showProcessingScreen();
}

#endif // DISPLAY_MANAGER_H
