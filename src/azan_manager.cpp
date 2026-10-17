#include "azan_manager.h"
#include "config.h"
#include <AudioFileSource.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <WiFiClientSecure.h>
#include <new>

// ============================================================
//  AZAN MANAGER IMPLEMENTATION
//  Generates a repeating two-beep prayer-time alert over I2S.
// ============================================================

namespace {
    class HttpsAudioSource : public AudioFileSource {
    public:
        bool open(const char* url) override {
            close();
            if (!url || strncmp(url, "https://", 8) != 0) return false;

            const char* hostStart = url + 8;
            const char* pathStart = strchr(hostStart, '/');
            if (!pathStart || pathStart == hostStart ||
                static_cast<size_t>(pathStart - hostStart) >= sizeof(host)) {
                return false;
            }
            const size_t hostLength = pathStart - hostStart;
            memcpy(host, hostStart, hostLength);
            host[hostLength] = '\0';

            client.setInsecure();
            client.setTimeout(1500);
            if (!client.connect(host, 443)) {
                Serial.println("[QURAN] Could not connect to mp3quran.net CDN.");
                return false;
            }
            client.printf("GET %s HTTP/1.1\r\nHost: %s\r\n"
                          "User-Agent: Prayer-Bot/1.0\r\n"
                          "Accept: audio/mpeg\r\nConnection: close\r\n\r\n",
                          pathStart, host);

            String statusLine = client.readStringUntil('\n');
            statusLine.trim();
            if (!statusLine.startsWith("HTTP/1.1 200") &&
                !statusLine.startsWith("HTTP/1.0 200")) {
                Serial.printf("[QURAN] CDN returned %s\n", statusLine.c_str());
                close();
                return false;
            }

            size = 0;
            bool chunked = false;
            while (client.connected()) {
                String header = client.readStringUntil('\n');
                header.trim();
                if (header.isEmpty()) break;
                if (header.startsWith("Content-Length:")) {
                    size = static_cast<uint32_t>(header.substring(15).toInt());
                } else if (header.equalsIgnoreCase("Transfer-Encoding: chunked")) {
                    chunked = true;
                }
            }
            if (chunked || (size == 0 && !client.connected())) {
                Serial.println("[QURAN] CDN response is not a supported MP3 stream.");
                close();
                return false;
            }
            position = 0;
            return true;
        }

        uint32_t read(void* data, uint32_t length) override {
            if (!data || (!client.connected() && client.available() == 0) ||
                (size && position >= size)) return 0;
            if (size && length > size - position) length = size - position;
            const size_t received = client.readBytes(
                static_cast<uint8_t*>(data), length);
            position += received;
            return static_cast<uint32_t>(received);
        }

        bool close() override {
            client.stop();
            position = 0;
            size = 0;
            return true;
        }

        bool isOpen() override {
            return client.connected() || client.available() > 0;
        }

        uint32_t getSize() override { return size; }
        uint32_t getPos() override { return position; }

    private:
        WiFiClientSecure client;
        char host[96] = {};
        uint32_t position = 0;
        uint32_t size = 0;
    };

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
    bool outputStarted = false;
    bool quranActive = false;
    bool quranStarting = false;
    volatile bool quranStopRequested = false;
    bool quranDecoderStarted = false;
    volatile int16_t quranPendingSurah = -1;
    volatile bool quranQueuedStartFailed = false;
    uint8_t currentVolume = DEFAULT_VOLUME;
    unsigned long alarmStartedMs = 0;
    unsigned long pomodoroBeepStartedMs = 0;
    uint32_t samplePhase = 0;

    AudioOutputI2S *out = nullptr;
    HttpsAudioSource *quranSource = nullptr;
    AudioGeneratorMP3 *quranDecoder = nullptr;

    bool isBeepOn(unsigned long elapsedMs) {
        uint32_t cyclePosition = elapsedMs % BEEP_CYCLE_MS;
        return cyclePosition < BEEP_LENGTH_MS ||
               (cyclePosition >= BEEP_LENGTH_MS + BEEP_GAP_MS &&
                cyclePosition < (BEEP_LENGTH_MS * 2) + BEEP_GAP_MS);
    }

    void cleanupQuran() {
        if (quranDecoder) {
            if (quranDecoderStarted) quranDecoder->stop();
            delete quranDecoder;
            quranDecoder = nullptr;
        }
        quranDecoderStarted = false;
        if (quranSource) {
            quranSource->close();
            delete quranSource;
            quranSource = nullptr;
        }
        outputStarted = false;
        quranActive = false;
        quranStopRequested = false;
    }

    bool startQuranStream(uint8_t surahNumber) {
        if (!audioReady || !out || surahNumber < 1 ||
            surahNumber > TOTAL_SURAHS || playing || quranActive ||
            quranStarting || quranDecoder || quranSource ||
            quranStopRequested) {
            return false;
        }

        quranStarting = true;
        char url[128];
        snprintf(url, sizeof(url),
                 "https://cdn.mp3quran.net/audio/maher-muaiqly/r1/%03u.mp3",
                 surahNumber);
        quranSource = new (std::nothrow) HttpsAudioSource();
        quranDecoder = new (std::nothrow) AudioGeneratorMP3();
        if (!quranSource || !quranDecoder) {
            Serial.println("[QURAN] Not enough memory to prepare playback.");
            quranStarting = false;
            cleanupQuran();
            return false;
        }
        if (!quranSource->open(url)) {
            snprintf(url, sizeof(url),
                     "https://cdn.islamic.network/quran/audio/128/ar.alafasy/%u.mp3",
                     surahNumber);
            Serial.printf("[QURAN] Retrying Surah %u from alternate CDN.\n",
                          surahNumber);
        }
        if (!quranSource->isOpen() && !quranSource->open(url)) {
            Serial.printf("[QURAN] Could not start Surah %u stream.\n", surahNumber);
            quranStarting = false;
            cleanupQuran();
            return false;
        }
        if (!outputStarted) {
            outputStarted = out->begin();
            if (!outputStarted) {
                Serial.println("[QURAN] Could not restart I2S output.");
                quranStarting = false;
                cleanupQuran();
                return false;
            }
        }
        if (!quranDecoder->begin(quranSource, out)) {
            Serial.printf("[QURAN] Could not start Surah %u stream.\n", surahNumber);
            quranStarting = false;
            cleanupQuran();
            return false;
        }
        out->SetGain(static_cast<float>(currentVolume) / 100.0f);
        quranDecoderStarted = true;
        outputStarted = true;
        quranActive = true;
        quranStarting = false;
        Serial.printf("[QURAN] Streaming Surah %u from mp3quran.net.\n", surahNumber);
        return true;
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
    outputStarted = audioReady;
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

    stopQuran();
    if (!outputStarted) {
        outputStarted = out->begin();
        if (!outputStarted) {
            Serial.println("[AZAN] Could not restart I2S output.");
            return false;
        }
    }
    Serial.printf("[AZAN] Starting prayer alarm for prayer %u.\n", (unsigned)prayer);
    samplePhase = 0;
    alarmStartedMs = millis();
    pomodoroBeepPending = false;
    pomodoroBeepActive = false;
    playing = true;
    return true;
}

bool playQuranSurah(uint8_t surahNumber) {
    if (!audioReady || !out || surahNumber < 1 || surahNumber > TOTAL_SURAHS) {
        Serial.println("[QURAN] Cannot start playback: invalid Surah or unavailable I2S.");
        return false;
    }
    if (playing) {
        Serial.println("[QURAN] Prayer alarm is active; Quran playback was not started.");
        return false;
    }
    quranQueuedStartFailed = false;
    if (quranActive || quranStarting || quranDecoder || quranSource ||
        quranStopRequested) {
        quranPendingSurah = surahNumber;
        if (quranActive || quranDecoder || quranSource)
            quranStopRequested = true;
        return true;
    }

    return startQuranStream(surahNumber);
}

void stopQuran() {
    quranPendingSurah = -1;
    quranStopRequested = true;
}

bool isQuranPlaying() {
    return quranActive && !quranStopRequested;
}

bool isQuranPending() {
    return quranPendingSurah > 0 || quranStarting;
}

bool takeQuranStartFailure() {
    const bool failed = quranQueuedStartFailed;
    quranQueuedStartFailed = false;
    return failed;
}

void processQuranQueue() {
    if (quranPendingSurah < 1 || playing || quranActive || quranStarting ||
        quranDecoder || quranSource || quranStopRequested) {
        return;
    }

    const uint8_t requestedSurah =
        static_cast<uint8_t>(quranPendingSurah);
    quranPendingSurah = -1;
    if (!startQuranStream(requestedSurah)) {
        quranQueuedStartFailed = true;
        Serial.printf("[QURAN] Queued Surah %u could not be started.\n", requestedSurah);
    }
}

void update() {
    if (!audioReady || !out) return;

    if (quranStopRequested && !quranStarting) {
        cleanupQuran();
        if (playing || pomodoroBeepPending) {
            outputStarted = out->begin();
        }
    }

    if (quranActive && quranDecoder) {
        if (!quranDecoder->loop()) {
            Serial.println("[QURAN] Surah stream finished.");
            quranStopRequested = true;
        }
        return;
    }

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
    if (!playing) {
        stopQuran();
        return;
    }
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
    if (!playing) {
        stopQuran();
        if (!outputStarted) outputStarted = out->begin();
        pomodoroBeepPending = outputStarted;
    }
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
