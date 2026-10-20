#ifndef AZAN_MANAGER_H
#define AZAN_MANAGER_H

#include "types.h"

// ============================================================
//  AZAN MANAGER
//  Manages azan audio playback via I2S. Handles Fajr vs
//  standard azan selection and playback state.
// ============================================================

namespace AzanManager {
    // Initialize I2S amplifier
    bool init();

    // Start the repeating beep alarm for the given prayer
    bool play(PrayerName prayer);

    // Play a Surah from the mp3quran.net Maher Al-Muaiqly stream.
    bool playQuranSurah(uint8_t surahNumber);

    // Stop Quran playback without affecting the prayer alarm state.
    void stopQuran();
    bool isQuranPlaying();
    bool isQuranPending();
    void processQuranQueue();
    bool takeQuranStartFailure();

    // Stop playback
    void stop();

    // Play one short notification beep
    void playPomodoroBeep();
    void playScreenChangeBeep();

    // Feed audio samples (call frequently in dedicated task)
    void update();

    // Is azan currently playing?
    bool isPlaying();

    // Set volume (0-100)
    void setVolume(uint8_t volume);
}

#endif // AZAN_MANAGER_H
