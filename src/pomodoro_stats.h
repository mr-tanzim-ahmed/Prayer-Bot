#ifndef POMODORO_STATS_H
#define POMODORO_STATS_H

#include <Arduino.h>
#include <ArduinoJson.h>

namespace PomodoroStats {
    bool init();
    bool recordSessionStarted();
    bool recordFocusCompleted(uint16_t focusMinutes);
    bool getWeeklyStats(JsonDocument& output);
}

#endif // POMODORO_STATS_H
