#include "button_handler.h"
#include "config.h"

// ============================================================
//  BUTTON HANDLER IMPLEMENTATION (CAPACITIVE TOUCH)
// ============================================================

namespace {
    bool touchStarted = false;
    bool touchPressed = false;
    bool touchLongPressed = false;
    bool previousTouchState = false;
    bool rawTouchState = false;
    bool longPressReported = false;
    unsigned long touchStartedMs = 0;
    unsigned long rawStateChangedMs = 0;
    constexpr unsigned long TOUCH_LONG_PRESS_MS = 3000UL;
}

namespace ButtonHandler {

void init() {
    pinMode(PIN_TOUCH_SENSOR, INPUT);
    previousTouchState = digitalRead(PIN_TOUCH_SENSOR) == HIGH;
    rawTouchState = previousTouchState;
    touchPressed = false;
    touchLongPressed = false;
    longPressReported = previousTouchState;
    touchStartedMs = 0;
    rawStateChangedMs = millis();
    Serial.println("[BUTTON] TTP223 active-HIGH digital input initialized.");
}

void update() {
    touchStarted = false;
    touchPressed = false;
    touchLongPressed = false;

    const unsigned long now = millis();
    const bool currentTouchState = digitalRead(PIN_TOUCH_SENSOR) == HIGH;
    if (currentTouchState != rawTouchState) {
        rawTouchState = currentTouchState;
        rawStateChangedMs = now;
    }
    if (rawTouchState != previousTouchState &&
        now - rawStateChangedMs >= TOUCH_DEBOUNCE_MS) {
        previousTouchState = rawTouchState;
        if (previousTouchState) {
            touchStarted = true;
            touchStartedMs = rawStateChangedMs;
            longPressReported = false;
            Serial.println("[BUTTON] Touch detected.");
        } else if (!longPressReported) {
            touchPressed = true;
            Serial.println("[BUTTON] Short touch released.");
        }
    }
    if (previousTouchState && !longPressReported &&
        now - touchStartedMs >= TOUCH_LONG_PRESS_MS) {
        touchLongPressed = true;
        longPressReported = true;
        Serial.println("[BUTTON] Three-second touch detected.");
    }
}

bool isTouchStarted() {
    return touchStarted;
}

bool isTouchPressed() {
    return touchPressed;
}

bool isTouchLongPressed() {
    return touchLongPressed;
}

} // namespace ButtonHandler
