#include "pomodoro_manager.h"
#include "config.h"

// ============================================================
//  POMODORO MANAGER IMPLEMENTATION
// ============================================================

namespace PomodoroManager {

void init(const Settings& settings) {
    // Defaults are set when first used via start()
    // Settings are read from the Settings struct
}

void update(PomodoroState& state) {
    if (state.phase == POMODORO_IDLE || state.phase == POMODORO_PAUSED) return;

    unsigned long now = millis();
    unsigned long elapsed = now - state.phaseStartMs;
    uint32_t phaseDurationMs = 0;

    switch (state.phase) {
        case POMODORO_FOCUS:
            phaseDurationMs = (uint32_t)state.focusMinutes * 60UL * 1000UL;
            break;
        case POMODORO_SHORT_BREAK:
            phaseDurationMs = (uint32_t)state.shortBreakMinutes * 60UL * 1000UL;
            break;
        case POMODORO_LONG_BREAK:
            phaseDurationMs = (uint32_t)state.longBreakMinutes * 60UL * 1000UL;
            break;
        default:
            return;
    }

    if (elapsed >= phaseDurationMs) {
        // Phase complete - transition
        switch (state.phase) {
            case POMODORO_FOCUS:
                state.currentCycle++;
                if (state.currentCycle >= state.totalCycles) {
                    state.phase = POMODORO_LONG_BREAK;
                    state.currentCycle = 0;
                } else {
                    state.phase = POMODORO_SHORT_BREAK;
                }
                break;

            case POMODORO_SHORT_BREAK:
            case POMODORO_LONG_BREAK:
                state.phase = POMODORO_FOCUS;
                break;

            default:
                break;
        }
        state.phaseStartMs = now;
        state.remainingSeconds = 0;
    } else {
        state.remainingSeconds = (int32_t)((phaseDurationMs - elapsed) / 1000UL);
    }
}

void start(PomodoroState& state) {
    state.phase = POMODORO_FOCUS;
    state.currentCycle = 0;
    state.phaseStartMs = millis();
    state.pausedElapsedMs = 0;
    state.pausedByAzan = false;
    state.remainingSeconds = state.focusMinutes * 60;
    Serial.println("[POMODORO] Started.");
}

void togglePause(PomodoroState& state) {
    if (state.phase == POMODORO_IDLE) {
        start(state);
        return;
    }

    if (state.phase == POMODORO_PAUSED) {
        // Resume
        state.phase = POMODORO_FOCUS; // TODO: restore previous phase
        state.phaseStartMs = millis() - state.pausedElapsedMs;
        state.pausedByAzan = false;
        Serial.println("[POMODORO] Resumed.");
    } else {
        // Pause
        state.pausedElapsedMs = millis() - state.phaseStartMs;
        state.phase = POMODORO_PAUSED;
        Serial.println("[POMODORO] Paused.");
    }
}

void reset(PomodoroState& state) {
    state.phase = POMODORO_IDLE;
    state.currentCycle = 0;
    state.remainingSeconds = 0;
    state.pausedElapsedMs = 0;
    state.pausedByAzan = false;
    Serial.println("[POMODORO] Reset.");
}

void pauseForAzan(PomodoroState& state) {
    if (state.phase != POMODORO_IDLE && state.phase != POMODORO_PAUSED) {
        state.pausedElapsedMs = millis() - state.phaseStartMs;
        state.phase = POMODORO_PAUSED;
        state.pausedByAzan = true;
        Serial.println("[POMODORO] Paused for azan.");
    }
}

void resumeFromAzan(PomodoroState& state) {
    if (state.pausedByAzan && state.phase == POMODORO_PAUSED) {
        state.phase = POMODORO_FOCUS; // TODO: restore previous phase
        state.phaseStartMs = millis() - state.pausedElapsedMs;
        state.pausedByAzan = false;
        Serial.println("[POMODORO] Resumed after azan.");
    }
}

void updateSettings(PomodoroState& state, const Settings& settings) {
    state.focusMinutes      = settings.focusMinutes;
    state.shortBreakMinutes = settings.shortBreakMinutes;
    state.longBreakMinutes  = settings.longBreakMinutes;
    state.totalCycles       = settings.pomodoroCycles;
    Serial.println("[POMODORO] Settings updated.");
}

} // namespace PomodoroManager
