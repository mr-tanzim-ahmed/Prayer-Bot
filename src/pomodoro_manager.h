#ifndef POMODORO_MANAGER_H
#define POMODORO_MANAGER_H

#include "types.h"

// ============================================================
//  POMODORO MANAGER
//  State machine for focus / short break / long break phases.
//  Integrates with azan (auto-pause during azan).
// ============================================================

namespace PomodoroManager {
    // Initialize with settings
    void init(const Settings& settings);

    // Update timer - call frequently
    void update(PomodoroState& state);

    // User controls
    void start(PomodoroState& state);
    void togglePause(PomodoroState& state);
    void reset(PomodoroState& state);

    // Azan integration
    void pauseForAzan(PomodoroState& state);
    void resumeFromAzan(PomodoroState& state);

    // Update settings (from dashboard)
    void updateSettings(PomodoroState& state, const Settings& settings);
}

#endif // POMODORO_MANAGER_H
