#ifndef SHAKE_DETECTOR_H
#define SHAKE_DETECTOR_H

#include <Arduino.h>

// ============================================================
//  SHAKE DETECTOR
//  Reads MPU6050 accelerometer data and detects shake
//  gestures to stop the azan.
// ============================================================

namespace ShakeDetector {
    // Initialize MPU6050
    bool init();

    // Read accelerometer and check for shake - call frequently
    void update();

    // Was a shake detected since last check? (auto-resets)
    bool isShakeDetected();

    // Reset shake state
    void reset();
}

#endif // SHAKE_DETECTOR_H
