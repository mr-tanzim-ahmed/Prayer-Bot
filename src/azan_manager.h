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
    void init();

    // Play azan for the given prayer (selects Fajr or standard)
    void play(PrayerName prayer);

    // Stop playback
    void stop();

    // Decode audio frame (call frequently in dedicated task)
    void update();

    // Is azan currently playing?
    bool isPlaying();

    // Set volume (0-100)
    void setVolume(uint8_t volume);
}

#endif // AZAN_MANAGER_H
