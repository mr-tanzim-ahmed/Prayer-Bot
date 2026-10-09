#include "azan_manager.h"
#include "config.h"
#include <LittleFS.h>
#include <AudioFileSourceLittleFS.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>

// ============================================================
//  AZAN MANAGER IMPLEMENTATION
//  Handles I2S MP3 decoding and playback from LittleFS.
// ============================================================

namespace {
    bool playing = false;
    uint8_t currentVolume = DEFAULT_VOLUME;

    AudioFileSourceLittleFS *file = nullptr;
    AudioGeneratorMP3 *mp3 = nullptr;
    AudioOutputI2S *out = nullptr;
}

namespace AzanManager {

void init() {
    out = new AudioOutputI2S(I2S_AMP_PORT, AudioOutputI2S::EXTERNAL_I2S);
    out->SetPinout(PIN_AMP_BCLK, PIN_AMP_LRC, PIN_AMP_DIN);
    out->SetGain((float)currentVolume / 100.0f);
    
    mp3 = new AudioGeneratorMP3();
    
    Serial.println("[AZAN] I2S amplifier and MP3 decoder initialized.");
}

void play(PrayerName prayer) {
    if (playing) stop();

    const char* azanFile = (prayer == PRAYER_FAJR) ? "/azan_fajr.mp3" : "/azan_standard.mp3";
    
    if (!LittleFS.exists(azanFile)) {
        Serial.printf("[AZAN] Error: File %s not found in LittleFS!\n", azanFile);
        return;
    }

    Serial.printf("[AZAN] Playing: %s\n", azanFile);

    file = new AudioFileSourceLittleFS(azanFile);
    
    out->SetGain((float)currentVolume / 100.0f);
    mp3->begin(file, out);
    
    playing = true;
}

void update() {
    if (playing && mp3 && mp3->isRunning()) {
        if (!mp3->loop()) {
            mp3->stop();
            stop();
        }
    }
}

void stop() {
    if (!playing) return;
    
    if (mp3 && mp3->isRunning()) {
        mp3->stop();
    }
    
    if (file) {
        file->close();
        delete file;
        file = nullptr;
    }
    
    playing = false;
    Serial.println("[AZAN] Stopped.");
}

bool isPlaying() {
    return playing;
}

void setVolume(uint8_t volume) {
    currentVolume = volume;
    if (out) {
        out->SetGain((float)volume / 100.0f);
    }
    Serial.printf("[AZAN] Volume set to: %d\n", volume);
}

} // namespace AzanManager
