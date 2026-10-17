#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// ============================================================
//  BUTTON HANDLER
//  Manages the TTP223B capacitive touch sensor.
// ============================================================

namespace ButtonHandler {
    // Initialize the touch sensor
    void init();

    // Update touch state - call every loop iteration
    void update();

    // True for one update when the sensor is newly touched
    bool isTouchPressed();

    // True once when the sensor has been held for three seconds
    bool isTouchLongPressed();
}

#endif // BUTTON_HANDLER_H
