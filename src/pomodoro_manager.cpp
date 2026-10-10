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
    if (state.phase == POMODORO_IDLE ||
        state.phase == POMODORO_PAUSED ||
        state.phase == POMODORO_WAITING_FOR_BREAK) return;

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
                state.pendingBreakPhase =
                    state.currentCycle >= state.totalCycles
                        ? POMODORO_LONG_BREAK
                        : POMODORO_SHORT_BREAK;
                state.phase = POMODORO_WAITING_FOR_BREAK;
                break;

            case POMODORO_SHORT_BREAK:
            case POMODORO_LONG_BREAK:
                if (state.phase == POMODORO_LONG_BREAK) {
                    state.currentCycle = 0;
                }
                state.phase = POMODORO_FOCUS;
                state.phaseStartMs = now;
                state.remainingSeconds = state.focusMinutes * 60;
                break;

            default:
                break;
        }
        if (state.phase == POMODORO_WAITING_FOR_BREAK) {
            state.phaseStartMs = 0;
            state.remainingSeconds =
                (int32_t)(state.pendingBreakPhase == POMODORO_LONG_BREAK
                    ? state.longBreakMinutes
                    : state.shortBreakMinutes) * 60;
        } else if (state.phase != POMODORO_FOCUS) {
            state.phaseStartMs = now;
            state.remainingSeconds = 0;
        }
    } else {
        state.remainingSeconds = (int32_t)((phaseDurationMs - elapsed) / 1000UL);
    }
}

void start(PomodoroState& state) {
    state.phase = POMODORO_FOCUS;
    state.savedPhase = POMODORO_FOCUS;
    state.currentCycle = 0;
    state.pendingBreakPhase = POMODORO_SHORT_BREAK;
    state.phaseStartMs = millis();
    state.pausedElapsedMs = 0;
    state.pausedByAzan = false;
    state.remainingSeconds = state.focusMinutes * 60;
    Serial.println("[POMODORO] Started.");
}

void startBreak(PomodoroState& state) {
    if (state.phase != POMODORO_WAITING_FOR_BREAK) return;
    state.phase = state.pendingBreakPhase;
    state.phaseStartMs = millis();
    state.pausedElapsedMs = 0;
    state.remainingSeconds = (int32_t)(
        state.phase == POMODORO_LONG_BREAK
            ? state.longBreakMinutes
            : state.shortBreakMinutes) * 60;
    Serial.println("[POMODORO] Break started by touch.");
}

void togglePause(PomodoroState& state) {
    if (state.phase == POMODORO_WAITING_FOR_BREAK) {
        startBreak(state);
        return;
    }

    if (state.phase == POMODORO_IDLE) {
        start(state);
        return;
    }

    if (state.phase == POMODORO_PAUSED) {
        // Resume to the phase we were in before pausing
        state.phase = state.savedPhase;
        state.phaseStartMs = millis() - state.pausedElapsedMs;
        state.pausedByAzan = false;
        Serial.println("[POMODORO] Resumed.");
    } else {
        // Pause - save current phase
        state.savedPhase = state.phase;
        state.pausedElapsedMs = millis() - state.phaseStartMs;
        state.phase = POMODORO_PAUSED;
        Serial.println("[POMODORO] Paused.");
    }
}

void reset(PomodoroState& state) {
    state.phase = POMODORO_IDLE;
    state.savedPhase = POMODORO_IDLE;
    state.currentCycle = 0;
    state.pendingBreakPhase = POMODORO_SHORT_BREAK;
    state.remainingSeconds = 0;
    state.pausedElapsedMs = 0;
    state.pausedByAzan = false;
    Serial.println("[POMODORO] Reset.");
}

void pauseForAzan(PomodoroState& state) {
    if (state.phase != POMODORO_IDLE && state.phase != POMODORO_PAUSED) {
        state.savedPhase = state.phase;
        state.pausedElapsedMs = millis() - state.phaseStartMs;
        state.phase = POMODORO_PAUSED;
        state.pausedByAzan = true;
        Serial.println("[POMODORO] Paused for azan.");
    }
}

void resumeFromAzan(PomodoroState& state) {
    if (state.pausedByAzan && state.phase == POMODORO_PAUSED) {
        state.phase = state.savedPhase;
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
