#ifndef I2C_BUS_H
#define I2C_BUS_H

#include <Arduino.h>

namespace I2CBus {
    bool init();
    bool lock(uint32_t timeoutMs = 100);
    void unlock();
}

#endif
