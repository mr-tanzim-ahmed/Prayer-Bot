#include "azan_manager.h"
#include "config.h"
#include <AudioOutputI2S.h>

// ============================================================
//  AZAN MANAGER IMPLEMENTATION
//  Generates a repeating two-beep prayer-time alert over I2S.
// ============================================================

namespace {
    constexpr uint32_t BEEP_FREQUENCY_HZ = 880;
    constexpr uint32_t BEEP_CYCLE_MS = 2000;
    constexpr uint32_t BEEP_LENGTH_MS = 250;
    constexpr uint32_t BEEP_GAP_MS = 200;
    constexpr uint32_t POMODORO_BEEP_LENGTH_MS = 250;
    constexpr int16_t BEEP_AMPLITUDE = 9000;
    constexpr size_t AUDIO_SAMPLES_PER_UPDATE = 256;

    volatile bool playing = false;
    volatile bool pomodoroBeepPending = false;
    bool pomodoroBeepActive = false;
    bool audioReady = false;
    uint8_t currentVolume = DEFAULT_VOLUME;
    unsigned long alarmStartedMs = 0;
    unsigned long pomodoroBeepStartedMs = 0;
    uint32_t samplePhase = 0;

    AudioOutputI2S *out = nullptr;

    bool isBeepOn(unsigned long elapsedMs) {
        uint32_t cyclePosition = elapsedMs % BEEP_CYCLE_MS;
        return cyclePosition < BEEP_LENGTH_MS ||
               (cyclePosition >= BEEP_LENGTH_MS + BEEP_GAP_MS &&
                cyclePosition < (BEEP_LENGTH_MS * 2) + BEEP_GAP_MS);
    }
}

namespace AzanManager {

bool init() {
    out = new AudioOutputI2S(I2S_AMP_PORT, AudioOutputI2S::EXTERNAL_I2S);
    if (!out->SetPinout(PIN_AMP_BCLK, PIN_AMP_LRC, PIN_AMP_DIN)) {
        Serial.println("[AZAN] Failed to configure I2S pins.");
        return false;
    }

    out->SetRate(AMP_SAMPLE_RATE);
    out->SetGain((float)currentVolume / 100.0f);
    audioReady = out->begin();
    if (!audioReady) {
        Serial.println("[AZAN] Failed to initialize I2S audio output.");
        return false;
    }

    Serial.println("[AZAN] I2S beep alarm initialized.");
    return true;
}

bool play(PrayerName prayer) {
    if (!audioReady) {
        Serial.println("[AZAN] Cannot start alarm: I2S audio is unavailable.");
        return false;
    }

    Serial.printf("[AZAN] Starting prayer alarm for prayer %u.\n", (unsigned)prayer);
    samplePhase = 0;
    alarmStartedMs = millis();
    pomodoroBeepPending = false;
    pomodoroBeepActive = false;
    playing = true;
    return true;
}

void update() {
    if (!audioReady || !out) return;

    if (pomodoroBeepPending && !playing) {
        pomodoroBeepPending = false;
        pomodoroBeepActive = true;
        pomodoroBeepStartedMs = millis();
        samplePhase = 0;
    }
    if (!playing && !pomodoroBeepActive) return;

    const unsigned long now = millis();
    if (pomodoroBeepActive &&
        now - pomodoroBeepStartedMs >= POMODORO_BEEP_LENGTH_MS) {
        pomodoroBeepActive = false;
    }
    const bool beepOn = playing
        ? isBeepOn(now - alarmStartedMs)
        : pomodoroBeepActive;
    const uint32_t phaseIncrement =
        (uint32_t)(((uint64_t)BEEP_FREQUENCY_HZ << 32) / AMP_SAMPLE_RATE);

    for (size_t i = 0; i < AUDIO_SAMPLES_PER_UPDATE; ++i) {
        int16_t value = 0;
        if (beepOn) {
            value = (samplePhase & 0x80000000UL) ? BEEP_AMPLITUDE : -BEEP_AMPLITUDE;
        }

        int16_t sample[2] = {value, value};
        if (!out->ConsumeSample(sample)) return;
        samplePhase += phaseIncrement;
    }
}

void stop() {
    if (!playing) return;
    playing = false;
    pomodoroBeepPending = false;
    pomodoroBeepActive = false;
    Serial.println("[AZAN] Alarm stopped.");
}

void playPomodoroBeep() {
    if (!audioReady) {
        Serial.println("[AZAN] Cannot play Pomodoro beep: I2S audio is unavailable.");
        return;
    }
    if (!playing) pomodoroBeepPending = true;
}

bool isPlaying() {
    return playing;
}

void setVolume(uint8_t volume) {
    currentVolume = volume > 100 ? 100 : volume;
    if (out) {
        out->SetGain((float)currentVolume / 100.0f);
    }
    Serial.printf("[AZAN] Volume set to: %d\n", currentVolume);
}

} // namespace AzanManager
