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

    bool touchPressed = false;
    bool touchWasActive = false;
    unsigned long touchLastDebounce = 0;
}

namespace ButtonHandler {

void init() {
    // Capacitive touch pins don't need pinMode() setup in Arduino core
    Serial.println("[BUTTON] Capacitive touch initialized.");
}

void update() {
    unsigned long now = millis();

    touchPressed = false;

    // The touch sensor reports a lower value while active on ESP32-S3.
    bool touched = touchRead(PIN_TOUCH_SENSOR) < TOUCH_THRESHOLD;
    if (touched && !touchWasActive &&
        (now - touchLastDebounce) >= TOUCH_DEBOUNCE_MS) {
        touchPressed = true;
        touchLastDebounce = now;
    }
    touchWasActive = touched;
}

bool isTouchPressed() {
    return touchPressed;
}

} // namespace ButtonHandler
