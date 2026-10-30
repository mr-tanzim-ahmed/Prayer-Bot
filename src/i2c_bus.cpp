#include "i2c_bus.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

namespace {
    SemaphoreHandle_t busMutex = nullptr;
}

namespace I2CBus {

bool init() {
    if (!busMutex) busMutex = xSemaphoreCreateMutex();
    return busMutex != nullptr;
}

bool lock(uint32_t timeoutMs) {
    return busMutex &&
        xSemaphoreTake(busMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

void unlock() {
    if (busMutex) xSemaphoreGive(busMutex);
}

}
