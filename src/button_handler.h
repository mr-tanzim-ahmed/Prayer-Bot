#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// ============================================================
//  BUTTON HANDLER
//  Manages the TTP223B capacitive touch sensor with debounce.
// ============================================================

namespace ButtonHandler {
    // Initialize the touch sensor
    void init();

    // Update touch state - call every loop iteration
    void update();

    // True for one update when the sensor is newly touched
    bool isTouchPressed();
}

#endif // BUTTON_HANDLER_H
