#ifndef SHARED_STATE_H
#define SHARED_STATE_H

#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t g_pomodoroMutex;
extern SemaphoreHandle_t g_prayerDataMutex;
extern SemaphoreHandle_t g_calendarMutex;

inline bool lockPomodoroState(TickType_t timeout = pdMS_TO_TICKS(50)) {
    return g_pomodoroMutex &&
        xSemaphoreTake(g_pomodoroMutex, timeout) == pdTRUE;
}

inline void unlockPomodoroState() {
    xSemaphoreGive(g_pomodoroMutex);
}

inline bool lockPrayerData(TickType_t timeout = pdMS_TO_TICKS(50)) {
    return g_prayerDataMutex &&
        xSemaphoreTake(g_prayerDataMutex, timeout) == pdTRUE;
}

inline void unlockPrayerData() {
    xSemaphoreGive(g_prayerDataMutex);
}

inline bool lockCalendarData(TickType_t timeout = pdMS_TO_TICKS(50)) {
    return g_calendarMutex &&
        xSemaphoreTake(g_calendarMutex, timeout) == pdTRUE;
}

inline void unlockCalendarData() {
    xSemaphoreGive(g_calendarMutex);
}

#endif
