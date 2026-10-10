#ifndef BUTTON_HANDLER_H
#define BUTTON_HANDLER_H

#include <Arduino.h>

// ============================================================
//  BUTTON HANDLER
//  Manages TTP223B capacitive touch buttons with debounce,
//  short press, and long press (5 sec) detection.
// ============================================================

namespace ButtonHandler {
    // Initialize button GPIOs
    void init();

    // Update button state - call every loop iteration
    void update();

    // Button 1: Screen cycle (short press only)
    bool isButton1Pressed();

    // Button 2: Context action (short press)
    bool isButton2ShortPressed();

    // Button 2: Voice mode (5-second hold)
    bool isButton2LongPressed();

    // True for one update after either touch pad is newly touched
    bool isAnyTouchPressed();

    // Whether button 2 is currently being held
    bool isButton2Held();
}

#endif // BUTTON_HANDLER_H
