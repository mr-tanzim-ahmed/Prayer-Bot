#include "button_handler.h"
#include "config.h"

// ============================================================
//  BUTTON HANDLER IMPLEMENTATION (CAPACITIVE TOUCH)
// ============================================================

namespace {
    // Touch threshold for ESP32-S3 (lower value means touched, usually drops below 40000 on S3, but Arduino API normalizes it. 
    // Typical un-touched is > 50000, touched is < 30000. Let's use a dynamic baseline or a fixed threshold of 30000.
    // Actually, on ESP32-S3 `touchRead()` returns raw values around 40000-60000. Touched is lower.
    const uint32_t TOUCH_THRESHOLD = 30000;

    // Button 1 state
    bool btn1_pressed = false;
    unsigned long btn1_lastDebounce = 0;

    // Button 2 state
    bool btn2_shortPressed = false;
    bool btn2_longPressed = false;
    bool btn2_isHeld = false;
    unsigned long btn2_pressStart = 0;
    unsigned long btn2_lastDebounce = 0;
    bool btn2_longFired = false;
}

namespace ButtonHandler {

void init() {
    // Capacitive touch pins don't need pinMode() setup in Arduino core
    Serial.println("[BUTTON] Capacitive touch initialized.");
}

void update() {
    unsigned long now = millis();

    btn1_pressed = false;
    btn2_shortPressed = false;
    btn2_longPressed = false;

    // --- Touch 1: Screen cycle (Short press) ---
    // touchRead returns a smaller value when touched on S3
    bool b1 = touchRead(PIN_BUTTON_1) < TOUCH_THRESHOLD;
    if (b1 && (now - btn1_lastDebounce) >= BUTTON_DEBOUNCE_MS) {
        btn1_pressed = true;
        btn1_lastDebounce = now;
    }

    // --- Touch 2: Short press + Long press (5 sec) ---
    bool b2 = touchRead(PIN_BUTTON_2) < TOUCH_THRESHOLD;

    if (b2) {
        if (!btn2_isHeld) {
            btn2_isHeld = true;
            btn2_pressStart = now;
            btn2_longFired = false;
        } else if (!btn2_longFired && (now - btn2_pressStart) >= BUTTON_LONG_PRESS_MS) {
            btn2_longPressed = true;
            btn2_longFired = true;
        }
    } else {
        if (btn2_isHeld) {
            if (!btn2_longFired && (now - btn2_lastDebounce) >= BUTTON_DEBOUNCE_MS) {
                btn2_shortPressed = true;
            }
            btn2_isHeld = false;
            btn2_lastDebounce = now;
        }
    }
}

bool isButton1Pressed() {
    return btn1_pressed;
}

bool isButton2ShortPressed() {
    return btn2_shortPressed;
}

bool isButton2LongPressed() {
    return btn2_longPressed;
}

} // namespace ButtonHandler
