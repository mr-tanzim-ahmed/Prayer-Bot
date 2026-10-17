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

    // Begin the three-second Surah-selection listening window.
    void startListening();

    void enterSurahScreen();
    void leaveSurahScreen();

    // Update state machine (recording -> processing -> playing)
    void update(VoiceState& state);
    void processPending(const Settings& settings);
    bool queueDashboardRecognition(const char* transcript, uint32_t& requestId);
    const char* getDashboardRecognition(uint32_t requestId,
                                        uint8_t& surahNumber);

    // Stop current playback
    void stopPlayback();

    // Surah queries
    uint8_t getRecognizedSurah();
    const char* getSurahName(uint8_t number);
    uint8_t getPlayingSurah();
    const char* getStatusText();

    // State queries
    bool isListening();
    bool isProcessing();
    bool isPlaying();
    bool hasError();

    // Surah navigation and manual control
    void selectSurah(int number);
    void nextSurah();
    void prevSurah();
    bool playSurah(int number);
}

#endif // VOICE_MANAGER_H
