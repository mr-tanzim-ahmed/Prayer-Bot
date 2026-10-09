#include "screens/pomodoro_screen.h"
#include "display_manager.h"
#include "config.h"

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
            case POMODORO_IDLE:
            default:                   return "READY";
        }
    }
}

namespace PomodoroScreen {

void draw(const PomodoroState& state) {
    U8G2& oled = DisplayManager::getDisplay();

    DisplayManager::clearBuffer();

    // Header
    oled.setFont(u8g2_font_6x10_tf);
    oled.drawStr(2, 9, "POMODORO");
    oled.drawHLine(0, 12, SCREEN_WIDTH);

    // Phase text
    oled.setFont(u8g2_font_7x14B_tf);
    DisplayManager::drawCenteredText(String(getPhaseText(state.phase)), 28);

    // Timer
    if (state.phase != POMODORO_IDLE) {
        oled.setFont(u8g2_font_logisoso20_tf);
        int m = state.remainingSeconds / 60;
        int s = state.remainingSeconds % 60;
        char buf[6];
        snprintf(buf, sizeof(buf), "%02d:%02d", m, s);
        DisplayManager::drawCenteredText(String(buf), 50);
    } else {
        oled.setFont(u8g2_font_6x10_tf);
        DisplayManager::drawCenteredText("Press to start", 45);
    }

    // Cycle counter
    oled.setFont(u8g2_font_5x8_tf);
    String cycleStr = String(state.currentCycle + 1) + "/" + String(state.totalCycles);
    oled.drawStr(2, 63, cycleStr.c_str());

    // Paused by azan indicator
    if (state.pausedByAzan) {
        oled.drawStr(40, 63, "(Azan)");
    }

    // Page indicator
    oled.drawStr(108, 63, "5/5");

    DisplayManager::sendBuffer();
}

void update(const PomodoroState& state) {
    draw(state);
}

} // namespace PomodoroScreen
