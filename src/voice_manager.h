#ifndef VOICE_MANAGER_H
#define VOICE_MANAGER_H

#include "types.h"

// ============================================================
//  VOICE MANAGER
//  Handles voice recording via INMP441, sending audio to
//  Gemini API for surah name recognition, and streaming
//  Quran audio playback.
// ============================================================

namespace VoiceManager {
    // Initialize I2S microphone
    void init();

    // Start 5-second recording
    void startListening();

    // Update state machine (recording -> processing -> playing)
    void update(VoiceState& state);

    // Stop current playback
    void stopPlayback();

    // Get recognized surah number (1-114, 0 if none)
    uint8_t getRecognizedSurah();

    // Get surah name by number
    const char* getSurahName(uint8_t number);
}

#endif // VOICE_MANAGER_H
