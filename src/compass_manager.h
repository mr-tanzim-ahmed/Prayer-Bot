#ifndef COMPASS_MANAGER_H
#define COMPASS_MANAGER_H

#include <Arduino.h>

struct CompassReading {
    float headingDegrees;
    bool headingValid;
    bool sensorAvailable;
    bool calibrated;
    bool calibrationSaveFailed;
    uint16_t calibrationSamples;
};

namespace CompassManager {
    bool init();
    void update();
    bool resetCalibration();
    void getReading(CompassReading& reading);
}

#endif
