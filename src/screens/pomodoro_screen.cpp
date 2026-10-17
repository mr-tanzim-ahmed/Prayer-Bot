#include "pomodoro_screen.h"
#include "../display_manager.h"
#include "../config.h"

// ============================================================
//  POMODORO SCREEN (Screen 5)
//  Focus/break timer, remaining time, phase, cycle count.
// ============================================================

namespace {
    const char* getPhaseText(PomodoroPhase phase) {
        switch (phase) {
            case POMODORO_FOCUS:       return "FOCUS";
            case POMODORO_SHORT_BREAK: return "SHORT BREAK";
            case POMODORO_LONG_BREAK:  return "LONG BREAK";
            case POMODORO_PAUSED:      return "PAUSED";
            case POMODORO_WAITING_FOR_BREAK: return "BREAK READY";
            case POMODORO_IDLE:
            default:                   return "READY";
        }
    }

    uint32_t getPhaseDurationSeconds(const PomodoroState& state) {
        PomodoroPhase phase = state.phase == POMODORO_PAUSED
            ? state.savedPhase
            : state.phase;
        switch (phase) {
            case POMODORO_FOCUS:
                return (uint32_t)state.focusMinutes * 60UL;
            case POMODORO_SHORT_BREAK:
                return (uint32_t)state.shortBreakMinutes * 60UL;
            case POMODORO_LONG_BREAK:
                return (uint32_t)state.longBreakMinutes * 60UL;
            default:
                return 0;
        }
    }
}

namespace PomodoroScreen {

void draw(const PomodoroState& state) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "POMODORO");
    String cycleStr = String(state.currentCycle + 1) + "/" + String(state.totalCycles);
    int cycleWidth = oled.getUTF8Width(cycleStr.c_str());
    oled.drawStr(SCREEN_WIDTH - cycleWidth - 3, 9, cycleStr.c_str());
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    const unsigned long animationFrame = (millis() / 180UL) % 8UL;
    const bool azanPaused = state.pausedByAzan;
    oled.setFont(u8g2_font_7x14B_tf);
    String phaseText = azanPaused ? "PAUSED FOR AZAN" : getPhaseText(state.phase);
    DisplayManager::drawCenteredText(phaseText, 27);

    if (state.phase == POMODORO_WAITING_FOR_BREAK) {
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Focus complete", 39);
        const int dotX = 15 + (int)(animationFrame * 12UL);
        oled.drawFrame(14, 44, 100, 5);
        oled.drawDisc(dotX, 46, 2);
        oled.drawStr(2, 62, "Tap break | Hold 3s reset");
    } else if (state.phase == POMODORO_IDLE) {
        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText("Ready to focus", 39);
        const int dotX = 15 + (int)(animationFrame * 12UL);
        oled.drawHLine(14, 45, 100);
        oled.drawDisc(dotX, 45, 3);
        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(2, 62, "Hold 3s start | Tap next");
    } else {
        oled.setFont(u8g2_font_logisoso20_tf);
        int m = state.remainingSeconds / 60;
        int s = state.remainingSeconds % 60;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", m, s);

        int timerWidth = oled.getUTF8Width(buf);
        int timerX = (SCREEN_WIDTH - timerWidth) / 2;
        oled.drawStr(timerX, 49, buf);

        const uint32_t phaseDuration = getPhaseDurationSeconds(state);
        int progressWidth = 0;
        if (phaseDuration > 0 && state.remainingSeconds >= 0) {
            uint32_t elapsed = state.remainingSeconds >= (int32_t)phaseDuration
                ? 0
                : phaseDuration - (uint32_t)state.remainingSeconds;
            progressWidth = (int)((elapsed * 96UL) / phaseDuration);
        }
        oled.drawFrame(14, 53, 100, 5);
        if (progressWidth > 0) {
            oled.drawBox(16, 55, progressWidth, 1);
        }

        oled.setFont(u8g2_font_5x8_tf);
        if (state.phase == POMODORO_PAUSED) {
            oled.drawStr(2, 62,
                         azanPaused ? "Paused for prayer" : "Tap resume + next");
        } else {
            const int pulseRadius = 1 + (int)(animationFrame % 3UL);
            oled.drawDisc(SCREEN_WIDTH - 8, 21, pulseRadius);
            oled.drawStr(2, 62, "Hold 3s pause | Tap next");
        }
    }

    oled.setFont(u8g2_font_5x8_tf);
    oled.drawStr(108, 62, "5/7");

    DisplayManager::sendBuffer();
}

void update(const PomodoroState& state) {
    draw(state);
}

} // namespace PomodoroScreen
