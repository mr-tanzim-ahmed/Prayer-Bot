#include "button_handler.h"
#include "config.h"

// ============================================================
//  BUTTON HANDLER IMPLEMENTATION
// ============================================================

namespace {
    // Button 1 state
    bool btn1_pressed = false;
    unsigned long btn1_lastDebounce = 0;

    // Button 2 state
    bool btn2_shortPressed = false;
    bool btn2_longPressed = false;
    bool btn2_isHeld = false;
    unsigned long btn2_pressStart = 0;
    unsigned long btn2_lastDebounce = 0;
    bool btn2_longFired = false;  // Prevent re-firing while held
}

namespace ButtonHandler {

void init() {
    pinMode(PIN_BUTTON_1, INPUT);
    pinMode(PIN_BUTTON_2, INPUT);
    Serial.println("[BUTTON] Initialized.");
}

void update() {
    unsigned long now = millis();

    // Reset previous frame's events
    btn1_pressed = false;
    btn2_shortPressed = false;
    btn2_longPressed = false;

    // --- Button 1: Short press only ---
    bool b1 = digitalRead(PIN_BUTTON_1) == HIGH;
    if (b1 && (now - btn1_lastDebounce) >= BUTTON_DEBOUNCE_MS) {
        btn1_pressed = true;
        btn1_lastDebounce = now;
    }

    // --- Button 2: Short press + Long press (5 sec) ---
    bool b2 = digitalRead(PIN_BUTTON_2) == HIGH;

    if (b2) {
        if (!btn2_isHeld) {
            // Just pressed
            btn2_isHeld = true;
            btn2_pressStart = now;
            btn2_longFired = false;
        } else if (!btn2_longFired && (now - btn2_pressStart) >= BUTTON_LONG_PRESS_MS) {
            // Held long enough
            btn2_longPressed = true;
            btn2_longFired = true;
        }
    } else {
        if (btn2_isHeld) {
            // Just released
            if (!btn2_longFired && (now - btn2_lastDebounce) >= BUTTON_DEBOUNCE_MS) {
                // Was a short press
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
