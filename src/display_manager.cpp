#include "display_manager.h"
#include <Wire.h>

// ============================================================
//  DISPLAY MANAGER IMPLEMENTATION
// ============================================================

namespace {
    bool oledAvailable = false;
    unsigned long lastOLEDCheck = 0;

    #if OLED_USE_SH1106
    U8G2_SH1106_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
    #else
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C oled(U8G2_R0, U8X8_PIN_NONE);
    #endif

    bool checkOLED() {
        Wire.beginTransmission(OLED_I2C_ADDRESS);
        return Wire.endTransmission() == 0;
    }
}

namespace DisplayManager {

bool init() {
    Wire.begin(PIN_OLED_SDA, PIN_OLED_SCL);
    Wire.setClock(I2C_CLOCK_SPEED);
    delay(20);

    if (!checkOLED()) {
        Serial.println("[OLED] Device not found at 0x3C");
        oledAvailable = false;
        return false;
    }

    oled.setI2CAddress(OLED_I2C_ADDRESS << 1);
    oled.begin();
    oled.setContrast(255);
    oled.clearBuffer();
    oled.sendBuffer();

    oledAvailable = true;
    lastOLEDCheck = millis();
    Serial.println("[OLED] Initialized.");
    return true;
}

bool isAvailable() {
    return oledAvailable;
}

void service() {
    unsigned long now = millis();
    if ((now - lastOLEDCheck) < OLED_CHECK_INTERVAL) return;
    lastOLEDCheck = now;

    if (oledAvailable) {
        if (!checkOLED()) {
            oledAvailable = false;
            Serial.println("[OLED] Connection lost.");
        }
        return;
    }

    // Try to reconnect
    Serial.println("[OLED] Trying to reconnect...");
    if (init()) {
        Serial.println("[OLED] Reconnected.");
    }
}

U8G2& getDisplay() {
    return oled;
}

void clearBuffer() {
    oled.clearBuffer();
    oled.setDrawColor(1);
}

void sendBuffer() {
    oled.sendBuffer();
}

void drawCenteredText(const String& text, int y) {
    int width = oled.getUTF8Width(text.c_str());
    int x = (SCREEN_WIDTH - width) / 2;
    if (x < 0) x = 0;
    oled.drawUTF8(x, y, text.c_str());
}

void drawText(const String& text, int x, int y) {
    oled.drawUTF8(x, y, text.c_str());
}

void drawHLine(int x, int y, int w) {
    oled.drawHLine(x, y, w);
}

void showStartupScreen() {
    if (!oledAvailable) return;
    clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    drawCenteredText("PRAYER-BOT", 16);
    oled.setFont(u8g2_font_5x8_tf);
    drawCenteredText("Islamic Personal Assistant", 31);
    drawCenteredText("Starting...", 46);
    sendBuffer();
}

void showConnectingScreen() {
    if (!oledAvailable) return;
    clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    drawCenteredText("Connecting WiFi...", 25);
    oled.setFont(u8g2_font_5x8_tf);
    drawCenteredText("Please wait", 42);
    sendBuffer();
}

void showErrorScreen(const String& line1, const String& line2) {
    if (!oledAvailable) return;
    clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    drawCenteredText(line1, 25);
    oled.setFont(u8g2_font_5x8_tf);
    drawCenteredText(line2, 42);
    sendBuffer();
}

void showListeningScreen() {
    if (!oledAvailable) return;
    clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    drawCenteredText("Listening...", 25);
    oled.setFont(u8g2_font_5x8_tf);
    drawCenteredText("Say a surah name", 42);
    sendBuffer();
}

void showProcessingScreen() {
    if (!oledAvailable) return;
    clearBuffer();
    oled.setFont(u8g2_font_6x10_tf);
    drawCenteredText("Processing...", 32);
    sendBuffer();
}

} // namespace DisplayManager
