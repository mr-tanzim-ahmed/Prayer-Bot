#include "button_handler.h"
#include "config.h"

// ============================================================
//  BUTTON HANDLER IMPLEMENTATION (CAPACITIVE TOUCH)
// ============================================================

namespace {
    bool touchPressed = false;
    bool touchLongPressed = false;
    bool previousTouchState = false;
    bool longPressReported = false;
    unsigned long touchStartedMs = 0;
    constexpr unsigned long TOUCH_LONG_PRESS_MS = 3000UL;
}

namespace ButtonHandler {

void init() {
    pinMode(PIN_TOUCH_SENSOR, INPUT);
    previousTouchState = false;
    touchPressed = false;
    touchLongPressed = false;
    longPressReported = false;
    touchStartedMs = 0;
    Serial.println("[BUTTON] TTP223 active-HIGH digital input initialized.");
}

void update() {
    touchPressed = false;
    touchLongPressed = false;

    bool currentTouchState = digitalRead(PIN_TOUCH_SENSOR) == HIGH;
    if (currentTouchState && !previousTouchState) {
        touchPressed = true;
        touchStartedMs = millis();
        longPressReported = false;
        Serial.println("[BUTTON] Touch detected.");
    } else if (currentTouchState && !longPressReported &&
               (millis() - touchStartedMs) >= TOUCH_LONG_PRESS_MS) {
        touchLongPressed = true;
        longPressReported = true;
        Serial.println("[BUTTON] Three-second touch detected.");
    }
    previousTouchState = currentTouchState;
}

bool isTouchPressed() {
    return touchPressed;
}

bool isTouchLongPressed() {
    return touchLongPressed;
}

} // namespace ButtonHandler
