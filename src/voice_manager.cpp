#include "voice_manager.h"
#include "config.h"
#include <driver/i2s.h>

// ============================================================
//  VOICE MANAGER IMPLEMENTATION
//  Records audio, sends to Gemini API for surah recognition,
//  and streams Quran recitation.
// ============================================================

namespace {
    uint8_t recognizedSurah = 0;
    bool micInitialized = false;

    // Surah names (first 10 shown, full list would have all 114)
    const char* SURAH_NAMES[] = {
        "Al-Fatiha", "Al-Baqarah", "Aal-E-Imran", "An-Nisa", "Al-Maidah",
        "Al-An'am", "Al-A'raf", "Al-Anfal", "At-Tawbah", "Yunus",
        "Hud", "Yusuf", "Ar-Ra'd", "Ibrahim", "Al-Hijr",
        "An-Nahl", "Al-Isra", "Al-Kahf", "Maryam", "Taha",
        "Al-Anbiya", "Al-Hajj", "Al-Mu'minun", "An-Nur", "Al-Furqan",
        "Ash-Shu'ara", "An-Naml", "Al-Qasas", "Al-Ankabut", "Ar-Rum",
        "Luqman", "As-Sajdah", "Al-Ahzab", "Saba", "Fatir",
        "Ya-Sin", "As-Saffat", "Sad", "Az-Zumar", "Ghafir",
        "Fussilat", "Ash-Shura", "Az-Zukhruf", "Ad-Dukhan", "Al-Jathiyah",
        "Al-Ahqaf", "Muhammad", "Al-Fath", "Al-Hujurat", "Qaf",
        "Adh-Dhariyat", "At-Tur", "An-Najm", "Al-Qamar", "Ar-Rahman",
        "Al-Waqi'ah", "Al-Hadid", "Al-Mujadila", "Al-Hashr", "Al-Mumtahanah",
        "As-Saf", "Al-Jumu'ah", "Al-Munafiqun", "At-Taghabun", "At-Talaq",
        "At-Tahrim", "Al-Mulk", "Al-Qalam", "Al-Haqqah", "Al-Ma'arij",
        "Nuh", "Al-Jinn", "Al-Muzzammil", "Al-Muddaththir", "Al-Qiyamah",
        "Al-Insan", "Al-Mursalat", "An-Naba", "An-Nazi'at", "Abasa",
        "At-Takwir", "Al-Infitar", "Al-Mutaffifin", "Al-Inshiqaq", "Al-Buruj",
        "At-Tariq", "Al-A'la", "Al-Ghashiyah", "Al-Fajr", "Al-Balad",
        "Ash-Shams", "Al-Lail", "Ad-Duha", "Ash-Sharh", "At-Tin",
        "Al-Alaq", "Al-Qadr", "Al-Bayyinah", "Az-Zalzalah", "Al-Adiyat",
        "Al-Qari'ah", "At-Takathur", "Al-Asr", "Al-Humazah", "Al-Fil",
        "Quraysh", "Al-Ma'un", "Al-Kawthar", "Al-Kafirun", "An-Nasr",
        "Al-Masad", "Al-Ikhlas", "Al-Falaq", "An-Nas"
    };
}

namespace VoiceManager {

void init() {
    // Configure I2S for microphone input
    i2s_config_t i2sConfig = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = MIC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024,
        .use_apll = false
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = PIN_MIC_SCK,
        .ws_io_num = PIN_MIC_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_MIC_SD
    };

    if (i2s_driver_install(I2S_MIC_PORT, &i2sConfig, 0, NULL) == ESP_OK) {
        i2s_set_pin(I2S_MIC_PORT, &pinConfig);
        micInitialized = true;
        Serial.println("[VOICE] I2S microphone initialized.");
    } else {
        Serial.println("[VOICE] I2S mic init failed!");
    }
}

void startListening() {
    if (!micInitialized) return;
    recognizedSurah = 0;
    Serial.println("[VOICE] Listening for surah name...");
    // TODO: Record MIC_RECORD_SECONDS of audio into PSRAM
    // Then send to Gemini API for recognition
}

void update(VoiceState& state) {
    switch (state) {
        case VOICE_LISTENING:
            // TODO: Check if recording is complete
            // When done, transition to VOICE_PROCESSING
            break;

        case VOICE_PROCESSING:
            // TODO: Check if Gemini API response received
            // When done, start playback and transition to VOICE_PLAYING
            break;

        case VOICE_PLAYING:
            // TODO: Check if playback is complete
            // When done, transition to VOICE_IDLE
            break;

        default:
            break;
    }
}

void stopPlayback() {
    // TODO: Stop streaming audio
    recognizedSurah = 0;
    Serial.println("[VOICE] Playback stopped.");
}

uint8_t getRecognizedSurah() {
    return recognizedSurah;
}

const char* getSurahName(uint8_t number) {
    if (number >= 1 && number <= TOTAL_SURAHS) {
        return SURAH_NAMES[number - 1];
    }
    return "Unknown";
}

} // namespace VoiceManager
