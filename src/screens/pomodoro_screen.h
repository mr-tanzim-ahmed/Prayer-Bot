#ifndef POMODORO_SCREEN_H
#define POMODORO_SCREEN_H

#include "types.h"

// ============================================================
//  POMODORO SCREEN (Screen 5)
//  Shows focus/break timer with remaining time, current phase,
//  and cycle count.
// ============================================================

namespace PomodoroScreen {
    void draw(const PomodoroState& state);
    void update(const PomodoroState& state);
}

#endif // POMODORO_SCREEN_H
