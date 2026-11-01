#include "azan_manager.h"
#include "config.h"
#include "https_client.h"
#include <ArduinoJson.h>
#include <AudioFileSource.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LittleFS.h>
#include <new>
#include <stdlib.h>

// ============================================================
//  AZAN MANAGER IMPLEMENTATION
//  Handles notification tones and Quran playback over I2S.
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

            configureHttpsClient(client);
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
    constexpr uint32_t SCREEN_CHANGE_BEEP_LENGTH_MS = 130;
    constexpr int16_t SCREEN_CHANGE_BEEP_AMPLITUDE = 9000;
    constexpr float SCREEN_CHANGE_BEEP_GAIN = 0.45f;
    constexpr float TOUCH_BEEP_GAIN = 0.65f;
    constexpr size_t AUDIO_SAMPLES_PER_UPDATE = 256;
    constexpr char TOUCH_BEEP_WAV_PATH[] = "/beep_sound.wav";

    volatile bool playing = false;
    volatile bool pomodoroBeepPending = false;
    volatile bool screenChangeBeepPending = false;
    volatile bool touchBeepPending = false;
    volatile bool touchBeepStopRequested = false;
    bool pomodoroBeepActive = false;
    bool screenChangeBeepActive = false;
    bool touchBeepActive = false;
    bool screenChangeGainReduced = false;
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
    unsigned long screenChangeBeepStartedMs = 0;
    uint32_t samplePhase = 0;

    AudioOutputI2S *out = nullptr;
    HttpsAudioSource *quranSource = nullptr;
    AudioGeneratorMP3 *quranDecoder = nullptr;
    File touchBeepFile;
    uint32_t touchBeepSampleRate = 0;
    uint32_t touchBeepDataBytesRemaining = 0;
    uint16_t touchBeepChannels = 0;
    uint16_t touchBeepBlockAlign = 0;
    int16_t touchBeepPendingLeft = 0;
    int16_t touchBeepPendingRight = 0;
    bool touchBeepSamplePending = false;

    bool startI2SOutput() {
        if (!out) return false;
        if (!outputStarted) outputStarted = out->begin();
        if (!outputStarted) return false;
        out->SetRate(AMP_SAMPLE_RATE);
        return true;
    }

    float getTouchBeepGain() {
        return static_cast<float>(currentVolume) / 100.0f *
               TOUCH_BEEP_GAIN;
    }

    uint16_t readLittleEndian16(const uint8_t* bytes) {
        return static_cast<uint16_t>(bytes[0]) |
               (static_cast<uint16_t>(bytes[1]) << 8);
    }

    uint32_t readLittleEndian32(const uint8_t* bytes) {
        return static_cast<uint32_t>(bytes[0]) |
               (static_cast<uint32_t>(bytes[1]) << 8) |
               (static_cast<uint32_t>(bytes[2]) << 16) |
               (static_cast<uint32_t>(bytes[3]) << 24);
    }

    void stopTouchBeep() {
        if (touchBeepFile) touchBeepFile.close();
        touchBeepActive = false;
        touchBeepDataBytesRemaining = 0;
        touchBeepSamplePending = false;
    }

    bool startTouchBeep() {
        touchBeepFile = LittleFS.open(TOUCH_BEEP_WAV_PATH, "r");
        if (!touchBeepFile || touchBeepFile.size() < 12) {
            Serial.printf("[AZAN] Touch sound %s is missing or empty; upload the LittleFS image.\n",
                          TOUCH_BEEP_WAV_PATH);
            stopTouchBeep();
            return false;
        }

        uint8_t riffHeader[12];
        if (touchBeepFile.read(riffHeader, sizeof(riffHeader)) !=
                sizeof(riffHeader) ||
            memcmp(riffHeader, "RIFF", 4) != 0 ||
            memcmp(riffHeader + 8, "WAVE", 4) != 0) {
            Serial.println("[AZAN] Touch sound is not a valid RIFF/WAVE file.");
            stopTouchBeep();
            return false;
        }

        const uint32_t fileSize = touchBeepFile.size();
        uint32_t offset = sizeof(riffHeader);
        uint32_t dataOffset = 0;
        uint16_t audioFormat = 0;
        uint16_t bitsPerSample = 0;
        bool formatFound = false;
        while (offset <= fileSize && fileSize - offset >= 8) {
            if (!touchBeepFile.seek(offset)) break;
            uint8_t chunkHeader[8];
            if (touchBeepFile.read(chunkHeader, sizeof(chunkHeader)) !=
                sizeof(chunkHeader)) {
                break;
            }

            const uint32_t chunkSize = readLittleEndian32(chunkHeader + 4);
            const uint32_t chunkDataOffset = offset + sizeof(chunkHeader);
            if (chunkDataOffset > fileSize ||
                chunkSize > fileSize - chunkDataOffset) {
                break;
            }

            if (memcmp(chunkHeader, "fmt ", 4) == 0) {
                uint8_t format[16];
                if (chunkSize < sizeof(format) ||
                    touchBeepFile.read(format, sizeof(format)) !=
                        sizeof(format)) {
                    break;
                }
                audioFormat = readLittleEndian16(format);
                touchBeepChannels = readLittleEndian16(format + 2);
                touchBeepSampleRate = readLittleEndian32(format + 4);
                touchBeepBlockAlign = readLittleEndian16(format + 12);
                bitsPerSample = readLittleEndian16(format + 14);
                formatFound = true;
            } else if (memcmp(chunkHeader, "data", 4) == 0) {
                dataOffset = chunkDataOffset;
                touchBeepDataBytesRemaining = chunkSize;
            }

            const uint32_t paddedChunkSize =
                chunkSize + (chunkSize & 1U);
            if (paddedChunkSize < chunkSize ||
                paddedChunkSize > fileSize - chunkDataOffset) {
                break;
            }
            offset = chunkDataOffset + paddedChunkSize;
        }

        if (!formatFound || !dataOffset || audioFormat != 1 ||
            bitsPerSample != 16 ||
            (touchBeepChannels != 1 && touchBeepChannels != 2) ||
            touchBeepSampleRate < 8000 || touchBeepSampleRate > 48000 ||
            touchBeepBlockAlign != touchBeepChannels * sizeof(int16_t) ||
            touchBeepDataBytesRemaining < touchBeepBlockAlign) {
            Serial.println("[AZAN] Touch WAV must contain 16-bit PCM mono or stereo audio.");
            stopTouchBeep();
            return false;
        }
        touchBeepDataBytesRemaining -=
            touchBeepDataBytesRemaining % touchBeepBlockAlign;
        if (!touchBeepFile.seek(dataOffset) ||
            !startI2SOutput() ||
            !out->SetRate(touchBeepSampleRate) ||
            !out->SetBitsPerSample(16) ||
            !out->SetChannels(2)) {
            Serial.println("[AZAN] Could not configure I2S for touch WAV playback.");
            outputStarted = false;
            stopTouchBeep();
            return false;
        }
        outputStarted = true;
        out->SetGain(getTouchBeepGain());
        touchBeepActive = true;
        Serial.printf("[AZAN] Playing touch WAV (%u Hz, %u channel(s)).\n",
                      static_cast<unsigned>(touchBeepSampleRate),
                      static_cast<unsigned>(touchBeepChannels));
        return true;
    }

    float getScreenChangeBeepGain() {
        return static_cast<float>(currentVolume) / 100.0f *
               SCREEN_CHANGE_BEEP_GAIN;
    }

    bool isBeepOn(unsigned long elapsedMs) {
        uint32_t cyclePosition = elapsedMs % BEEP_CYCLE_MS;
        return cyclePosition < BEEP_LENGTH_MS ||
               (cyclePosition >= BEEP_LENGTH_MS + BEEP_GAP_MS &&
                cyclePosition < (BEEP_LENGTH_MS * 2) + BEEP_GAP_MS);
    }

    bool serverHasSurah(const char* surahList, uint8_t surahNumber) {
        if (!surahList) return false;
        const char* current = surahList;
        while (*current) {
            char* end = nullptr;
            const long number = strtol(current, &end, 10);
            if (end == current) return false;
            if (number == surahNumber) return true;
            if (*end != ',') return false;
            current = end + 1;
        }
        return false;
    }

    bool resolveQuranStreamUrl(uint8_t surahNumber, char* url,
                               size_t urlCapacity) {
        const String apiUrl = String(MP3QURAN_API_BASE) +
            "/reciters?language=ar&reciter=102";
        WiFiClientSecure client;
        configureHttpsClient(client);

        HTTPClient http;
        http.setTimeout(10000);
        if (!http.begin(client, apiUrl)) {
            Serial.println("[QURAN] Could not initialize MP3Quran API request.");
            return false;
        }

        const int status = http.GET();
        if (status != HTTP_CODE_OK) {
            Serial.printf("[QURAN] MP3Quran API returned HTTP %d.\n", status);
            http.end();
            return false;
        }

        JsonDocument response;
        const DeserializationError parseError =
            deserializeJson(response, http.getStream());
        http.end();
        if (parseError) {
            Serial.printf("[QURAN] Could not parse MP3Quran API response: %s\n",
                          parseError.c_str());
            return false;
        }

        JsonArrayConst reciters = response["reciters"].as<JsonArrayConst>();
        for (JsonObjectConst reciter : reciters) {
            if ((reciter["id"] | 0) != 102) continue;
            JsonArrayConst recordings =
                reciter["moshaf"].as<JsonArrayConst>();
            for (JsonObjectConst recording : recordings) {
                if ((recording["rewaya_id"] | 0) != 1 ||
                    !serverHasSurah(recording["surah_list"] | "",
                                    surahNumber)) {
                    continue;
                }

                const char* server = recording["server"] | "";
                constexpr char TRUSTED_SERVER_PREFIX[] =
                    "https://cdn.mp3quran.net/audio/maher-muaiqly/";
                if (strncmp(server, TRUSTED_SERVER_PREFIX,
                            sizeof(TRUSTED_SERVER_PREFIX) - 1) != 0) {
                    Serial.println("[QURAN] MP3Quran API returned an unsupported audio server.");
                    return false;
                }

                const int length = snprintf(url, urlCapacity, "%s%03u.mp3",
                                            server, surahNumber);
                if (length < 0 ||
                    static_cast<size_t>(length) >= urlCapacity) {
                    Serial.println("[QURAN] MP3Quran stream URL is too long.");
                    return false;
                }
                return true;
            }
        }

        Serial.printf("[QURAN] MP3Quran API has no Hafs recording for Surah %u.\n",
                      surahNumber);
        return false;
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
            touchBeepPending || touchBeepActive || screenChangeBeepActive ||
            screenChangeBeepPending || pomodoroBeepPending ||
            pomodoroBeepActive ||
            quranStopRequested) {
            return false;
        }

        quranStarting = true;
        char url[180];
        if (!resolveQuranStreamUrl(surahNumber, url, sizeof(url))) {
            snprintf(url, sizeof(url),
                     "https://cdn.mp3quran.net/audio/maher-muaiqly/r1/%03u.mp3",
                     surahNumber);
            Serial.printf("[QURAN] Using validated direct stream fallback for Surah %u.\n",
                          surahNumber);
        }
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
    out = new (std::nothrow) AudioOutputI2S(
        I2S_AMP_PORT, AudioOutputI2S::EXTERNAL_I2S);
    if (!out) {
        Serial.println("[AZAN] Failed to allocate I2S audio output.");
        return false;
    }
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
    touchBeepStopRequested = true;
    if (!startI2SOutput()) {
        Serial.println("[AZAN] Could not restart I2S output.");
        return false;
    }
    Serial.printf("[AZAN] Starting prayer alarm for prayer %u.\n", (unsigned)prayer);
    samplePhase = 0;
    alarmStartedMs = millis();
    pomodoroBeepPending = false;
    pomodoroBeepActive = false;
    screenChangeBeepPending = false;
    screenChangeBeepActive = false;
    touchBeepPending = false;
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
    if (touchBeepPending || touchBeepActive ||
        screenChangeBeepPending || screenChangeBeepActive ||
        pomodoroBeepPending || pomodoroBeepActive) {
        quranPendingSurah = surahNumber;
        return true;
    }
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
        quranDecoder || quranSource || touchBeepPending ||
        touchBeepActive || screenChangeBeepPending || screenChangeBeepActive ||
        pomodoroBeepPending || pomodoroBeepActive || quranStopRequested) {
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
        if (playing || pomodoroBeepPending || screenChangeBeepPending ||
            touchBeepPending) {
            outputStarted = out->begin();
        }
    }

    if (touchBeepStopRequested) {
        touchBeepStopRequested = false;
        if (touchBeepFile) {
            stopTouchBeep();
            out->SetGain(static_cast<float>(currentVolume) / 100.0f);
            if (playing && !startI2SOutput()) {
                Serial.println("[AZAN] Could not restart I2S after interrupting touch WAV.");
            }
        }
    }

    if (quranActive && quranDecoder) {
        if (!quranDecoder->loop()) {
            Serial.println("[QURAN] Surah stream finished.");
            quranStopRequested = true;
        }
        return;
    }

    if (touchBeepActive) {
        for (size_t i = 0; i < AUDIO_SAMPLES_PER_UPDATE; ++i) {
            if (!touchBeepSamplePending) {
                if (touchBeepDataBytesRemaining < touchBeepBlockAlign) {
                    Serial.println("[AZAN] Touch WAV playback finished.");
                    stopTouchBeep();
                    out->SetRate(AMP_SAMPLE_RATE);
                    out->SetGain(static_cast<float>(currentVolume) / 100.0f);
                    break;
                }

                uint8_t sampleBytes[4];
                if (touchBeepFile.read(sampleBytes, touchBeepBlockAlign) !=
                    touchBeepBlockAlign) {
                    Serial.println("[AZAN] Could not read the next touch WAV sample.");
                    stopTouchBeep();
                    out->SetRate(AMP_SAMPLE_RATE);
                    out->SetGain(static_cast<float>(currentVolume) / 100.0f);
                    break;
                }
                touchBeepPendingLeft = static_cast<int16_t>(
                    readLittleEndian16(sampleBytes));
                touchBeepPendingRight = touchBeepChannels == 1
                    ? touchBeepPendingLeft
                    : static_cast<int16_t>(
                        readLittleEndian16(sampleBytes + sizeof(int16_t)));
                touchBeepSamplePending = true;
            }

            int16_t sample[2] = {
                touchBeepPendingLeft, touchBeepPendingRight
            };
            if (!out->ConsumeSample(sample)) return;
            touchBeepDataBytesRemaining -= touchBeepBlockAlign;
            touchBeepSamplePending = false;
        }
        if (touchBeepActive) return;
    }

    if (touchBeepPending && !playing && !pomodoroBeepActive &&
        !screenChangeBeepActive && !touchBeepActive &&
        !quranActive && !quranStarting && !quranDecoder && !quranSource &&
        !quranStopRequested) {
        touchBeepPending = false;
        startTouchBeep();
    } else if (screenChangeBeepPending && !playing && !pomodoroBeepActive &&
        !screenChangeBeepActive) {
        screenChangeBeepPending = false;
        out->SetGain(getScreenChangeBeepGain());
        screenChangeGainReduced = true;
        screenChangeBeepActive = true;
        screenChangeBeepStartedMs = millis();
        samplePhase = 0;
    } else if (pomodoroBeepPending && !playing && !screenChangeBeepActive &&
               !touchBeepActive) {
        pomodoroBeepPending = false;
        pomodoroBeepActive = true;
        pomodoroBeepStartedMs = millis();
        samplePhase = 0;
    }
    if (!playing && !pomodoroBeepActive && !screenChangeBeepActive) return;

    const unsigned long now = millis();
    if (pomodoroBeepActive &&
        now - pomodoroBeepStartedMs >= POMODORO_BEEP_LENGTH_MS) {
        pomodoroBeepActive = false;
    }
    if (screenChangeBeepActive &&
        now - screenChangeBeepStartedMs >= SCREEN_CHANGE_BEEP_LENGTH_MS) {
        screenChangeBeepActive = false;
    }
    if (screenChangeGainReduced && !screenChangeBeepActive &&
        !screenChangeBeepPending && !touchBeepActive &&
        !touchBeepPending) {
        out->SetGain(static_cast<float>(currentVolume) / 100.0f);
        screenChangeGainReduced = false;
    }
    const bool beepOn = playing
        ? isBeepOn(now - alarmStartedMs)
        : (pomodoroBeepActive || screenChangeBeepActive);
    const int16_t amplitude = screenChangeBeepActive
        ? SCREEN_CHANGE_BEEP_AMPLITUDE : BEEP_AMPLITUDE;
    const uint32_t phaseIncrement =
        (uint32_t)(((uint64_t)BEEP_FREQUENCY_HZ << 32) / AMP_SAMPLE_RATE);

    for (size_t i = 0; i < AUDIO_SAMPLES_PER_UPDATE; ++i) {
        int16_t value = 0;
        if (beepOn) {
            value = (samplePhase & 0x80000000UL) ? amplitude : -amplitude;
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
    screenChangeBeepPending = false;
    screenChangeBeepActive = false;
    touchBeepPending = false;
    Serial.println("[AZAN] Alarm stopped.");
}

void playPomodoroBeep() {
    if (!audioReady) {
        Serial.println("[AZAN] Cannot play Pomodoro beep: I2S audio is unavailable.");
        return;
    }
    if (!playing) {
        stopQuran();
        if (!startI2SOutput()) return;
        pomodoroBeepPending = outputStarted;
    }
}

void playScreenChangeBeep() {
    if (!audioReady) {
        Serial.println("[AZAN] Cannot play screen-change beep: I2S audio is unavailable.");
        return;
    }
    if (playing) return;

    stopQuran();
    if (!startI2SOutput()) {
        Serial.println("[AZAN] Could not restart I2S output for screen-change beep.");
        return;
    }
    screenChangeBeepPending = true;
}

void playTouchBeep() {
    if (!audioReady) {
        Serial.println("[AZAN] Touch feedback unavailable: I2S audio is not initialized.");
        return;
    }
    if (playing || quranActive || quranStarting) return;
    touchBeepPending = true;
}

bool isPlaying() {
    return playing;
}

void setVolume(uint8_t volume) {
    currentVolume = volume > 100 ? 100 : volume;
    if (out) {
        const float gain = touchBeepActive
            ? getTouchBeepGain()
            : (screenChangeBeepActive ? getScreenChangeBeepGain()
                                      : static_cast<float>(currentVolume) / 100.0f);
        out->SetGain(gain);
    }
    Serial.printf("[AZAN] Volume set to: %d\n", currentVolume);
}

} // namespace AzanManager
