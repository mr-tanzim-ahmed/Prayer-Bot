#include "pomodoro_screen.h"
#include "../display_manager.h"
#include "../config.h"
#include "screen_ui.h"

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

    String cycleStr = String(state.currentCycle + 1) + "/" + String(state.totalCycles);
    ScreenUI::drawHeader(oled, "FOCUS TIMER", cycleStr.c_str());

    const unsigned long animationFrame = (millis() / 350UL) % 8UL;
    const bool azanPaused = state.pausedByAzan;
    oled.setFont(u8g2_font_7x14B_tf);
    String phaseText = azanPaused ? "PAUSED FOR AZAN" : getPhaseText(state.phase);
    DisplayManager::drawCenteredText(phaseText, 29);

    if (state.phase == POMODORO_WAITING_FOR_BREAK) {
        oled.setFont(u8g2_font_5x8_tf);
        ScreenUI::drawCard(oled, 5, 33, SCREEN_WIDTH - 10, 21);
        DisplayManager::drawCenteredText("Beautiful work!", 44);
        const int dotX = 15 + (int)(animationFrame * 12UL);
        oled.drawHLine(15, 51, 98);
        oled.drawDisc(dotX, 51, 2);
        DisplayManager::drawCenteredText("Tap break  ·  Hold reset", 63);
    } else if (state.phase == POMODORO_IDLE) {
        oled.setFont(u8g2_font_6x10_tf);
        ScreenUI::drawCard(oled, 5, 33, SCREEN_WIDTH - 10, 21);
        DisplayManager::drawCenteredText("Ready when you are", 44);
        const int dotX = 15 + (int)(animationFrame * 12UL);
        oled.drawHLine(15, 51, 98);
        oled.drawDisc(dotX, 51, 2);
        oled.setFont(u8g2_font_5x8_tf);
        DisplayManager::drawCenteredText("Hold start  ·  Tap next", 63);
    } else {
        ScreenUI::drawCard(oled, 5, 32, SCREEN_WIDTH - 10, 27);
        oled.setFont(u8g2_font_logisoso20_tf);
        int m = state.remainingSeconds / 60;
        int s = state.remainingSeconds % 60;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", m, s);

        int timerWidth = oled.getStrWidth(buf);
        int timerX = (SCREEN_WIDTH - timerWidth) / 2;
        oled.drawStr(timerX, 51, buf);

        const uint32_t phaseDuration = getPhaseDurationSeconds(state);
        int progressWidth = 0;
        if (phaseDuration > 0 && state.remainingSeconds >= 0) {
            uint32_t elapsed = state.remainingSeconds >= (int32_t)phaseDuration
                ? 0
                : phaseDuration - (uint32_t)state.remainingSeconds;
            progressWidth = (int)((elapsed * 96UL) / phaseDuration);
        }
        ScreenUI::drawProgress(oled, 15, 53, 98, progressWidth, 96);

        oled.setFont(u8g2_font_5x8_tf);
        if (state.phase == POMODORO_PAUSED) {
            DisplayManager::drawCenteredText(
                azanPaused ? "Paused for prayer" : "Tap resume + next", 63);
        } else {
            DisplayManager::drawCenteredText("Hold to pause  ·  Tap next", 63);
        }
    }

    DisplayManager::sendBuffer();
}

void update(const PomodoroState& state) {
    draw(state);
}

} // namespace PomodoroScreen
