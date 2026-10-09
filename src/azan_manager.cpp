#include "azan_manager.h"
#include "config.h"
#include <driver/i2s.h>

// ============================================================
//  AZAN MANAGER IMPLEMENTATION
//  Handles I2S audio output for azan playback from LittleFS.
// ============================================================

namespace {
    bool playing = false;
    uint8_t currentVolume = DEFAULT_VOLUME;

    // TODO: Implement actual MP3 decoding and I2S streaming
    // This requires an MP3 decoder library (e.g., ESP8266Audio)
    // For now, this is a structural placeholder.
}

namespace AzanManager {

void init() {
    // Configure I2S for amplifier output
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = AMP_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false,
        .tx_desc_auto_clear = true
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = PIN_AMP_BCLK,
        .ws_io_num = PIN_AMP_LRC,
        .data_out_num = PIN_AMP_DIN,
        .data_in_num = I2S_PIN_NO_CHANGE
    };

    i2s_driver_install(I2S_AMP_PORT, &i2sConfig, 0, NULL);
    i2s_set_pin(I2S_AMP_PORT, &pinConfig);
    i2s_zero_dma_buffer(I2S_AMP_PORT);

    Serial.println("[AZAN] I2S amplifier initialized.");
}

void play(PrayerName prayer) {
    if (playing) return;

    const char* azanFile = (prayer == PRAYER_FAJR) ? "/azan_fajr.mp3" : "/azan_standard.mp3";
    Serial.printf("[AZAN] Playing: %s\n", azanFile);

    // TODO: Open file from LittleFS, decode MP3, stream to I2S
    // This needs an MP3 decoder library integration
    // Placeholder: set state to playing
    playing = true;
}

void stop() {
    if (!playing) return;
    i2s_zero_dma_buffer(I2S_AMP_PORT);
    playing = false;
    Serial.println("[AZAN] Stopped.");
}

bool isPlaying() {
    return playing;
}

void setVolume(uint8_t volume) {
    currentVolume = volume;
    // TODO: Apply volume scaling to audio output
    Serial.printf("[AZAN] Volume set to: %d\n", volume);
}

} // namespace AzanManager
