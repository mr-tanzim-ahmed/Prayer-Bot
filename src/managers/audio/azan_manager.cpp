#include "managers/audio/azan_manager.h"
#include <prayer_bot_config.h>
#include "network/https_client.h"
#include <ArduinoJson.h>
#include <AudioFileSource.h>
#include <AudioGeneratorMP3.h>
#include <AudioOutputI2S.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <math.h>
#include <new>
#include <stdlib.h>
#include <freertos/FreeRTOS.h>

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
                String normalizedHeader = header;
                normalizedHeader.toLowerCase();
                if (normalizedHeader.startsWith("content-length:")) {
                    size = static_cast<uint32_t>(header.substring(15).toInt());
                } else if (normalizedHeader.startsWith("transfer-encoding:") &&
                           normalizedHeader.indexOf("chunked") >= 0) {
                    chunked = true;
                }
            }
            if (size == 0 && !chunked && !client.connected()) {
                Serial.println("[QURAN] CDN response is not a supported MP3 stream.");
                close();
                return false;
            }
            chunkedTransfer = chunked;
            position = 0;
            return true;
        }

        uint32_t read(void* data, uint32_t length) override {
            if (!data || length == 0 || chunkFinished) return 0;
            if (chunkedTransfer) {
                while (chunkRemaining == 0) {
                    if (!readNextChunk()) return 0;
                }
                if (length > chunkRemaining) length = chunkRemaining;
            }
            if (!data || (!client.connected() && client.available() == 0) ||
                (size && position >= size)) return 0;
            if (size && length > size - position) length = size - position;
            const size_t received = client.readBytes(
                static_cast<uint8_t*>(data), length);
            position += received;
            if (chunkedTransfer) chunkRemaining -= received;
            return static_cast<uint32_t>(received);
        }

        bool close() override {
            client.stop();
            position = 0;
            size = 0;
            chunkRemaining = 0;
            chunkedTransfer = false;
            chunkStarted = false;
            chunkFinished = false;
            return true;
        }

        bool isOpen() override {
            return client.connected() || client.available() > 0;
        }

        uint32_t getSize() override { return size; }
        uint32_t getPos() override { return position; }

    private:
        bool readNextChunk() {
            if (chunkStarted) {
                char terminator[2];
                if (client.readBytes(reinterpret_cast<uint8_t*>(terminator),
                                     sizeof(terminator)) != sizeof(terminator) ||
                    terminator[0] != '\r' || terminator[1] != '\n') {
                    Serial.println("[QURAN] Invalid chunked audio stream.");
                    chunkFinished = true;
                    return false;
                }
            }

            String line = client.readStringUntil('\n');
            line.trim();
            const int extension = line.indexOf(';');
            if (extension >= 0) line.remove(extension);
            char* end = nullptr;
            const unsigned long chunkSize = strtoul(line.c_str(), &end, 16);
            if (line.isEmpty() || end == line.c_str() || *end != '\0' ||
                chunkSize > 1024UL * 1024UL) {
                Serial.println("[QURAN] Invalid chunk size in audio stream.");
                chunkFinished = true;
                return false;
            }
            if (chunkSize == 0) {
                while (client.connected()) {
                    String trailer = client.readStringUntil('\n');
                    trailer.trim();
                    if (trailer.isEmpty()) break;
                }
                chunkFinished = true;
                return false;
            }

            chunkRemaining = static_cast<uint32_t>(chunkSize);
            chunkStarted = true;
            return true;
        }

        WiFiClientSecure client;
        char host[96] = {};
        uint32_t position = 0;
        uint32_t size = 0;
        uint32_t chunkRemaining = 0;
        bool chunkedTransfer = false;
        bool chunkStarted = false;
        bool chunkFinished = false;
    };

    constexpr uint32_t BEEP_FREQUENCY_HZ = 880;
    constexpr uint32_t BEEP_CYCLE_MS = 2000;
    constexpr uint32_t BEEP_LENGTH_MS = 250;
    constexpr uint32_t BEEP_GAP_MS = 200;
    constexpr uint32_t POMODORO_BEEP_LENGTH_MS = 250;
    constexpr uint32_t TOUCH_BEEP_LENGTH_MS = 220;
    constexpr int16_t BEEP_AMPLITUDE = 9000;
    constexpr int16_t TOUCH_BEEP_AMPLITUDE = 20000;
    constexpr uint32_t SCREEN_CHANGE_BEEP_LENGTH_MS = 130;
    constexpr int16_t SCREEN_CHANGE_BEEP_AMPLITUDE = 9000;
    constexpr float SCREEN_CHANGE_BEEP_GAIN = 0.45f;
    constexpr float TOUCH_BEEP_GAIN = 0.65f;
    constexpr size_t AUDIO_SAMPLES_PER_UPDATE = 256;
    constexpr float AUDIO_TWO_PI = 6.28318530718f;
    constexpr float PHASE_SCALE = AUDIO_TWO_PI / 4294967296.0f;
    constexpr uint16_t BEEP_FADE_STEP = 150;

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
    portMUX_TYPE quranStateMux = portMUX_INITIALIZER_UNLOCKED;
    bool quranActive = false;
    bool quranStarting = false;
    bool quranStopRequested = false;
    bool quranDecoderStarted = false;
    int16_t quranPendingSurah = -1;
    bool quranQueuedStartFailed = false;
    uint32_t quranRequestGeneration = 0;
    uint32_t quranPreparedGeneration = 0;
    uint8_t currentVolume = DEFAULT_VOLUME;
    unsigned long alarmStartedMs = 0;
    unsigned long pomodoroBeepStartedMs = 0;
    unsigned long screenChangeBeepStartedMs = 0;
    unsigned long touchBeepStartedMs = 0;
    uint32_t samplePhase = 0;
    uint16_t beepEnvelope = 0;

    AudioOutputI2S *out = nullptr;
    // The active stream is owned by update(); the prepared pair is a locked handoff.
    HttpsAudioSource *quranSource = nullptr;
    AudioGeneratorMP3 *quranDecoder = nullptr;
    HttpsAudioSource *quranPreparedSource = nullptr;
    AudioGeneratorMP3 *quranPreparedDecoder = nullptr;

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

        JsonDocument filter;
        filter["reciters"][0]["id"] = true;
        filter["reciters"][0]["moshaf"][0]["rewaya_id"] = true;
        filter["reciters"][0]["moshaf"][0]["surah_list"] = true;
        filter["reciters"][0]["moshaf"][0]["server"] = true;
        JsonDocument response;
        const DeserializationError parseError = deserializeJson(
            response, http.getStream(), DeserializationOption::Filter(filter),
            DeserializationOption::NestingLimit(8));
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
        portENTER_CRITICAL(&quranStateMux);
        quranActive = false;
        portEXIT_CRITICAL(&quranStateMux);
    }

    bool prepareQuranStream(uint8_t surahNumber,
                            HttpsAudioSource*& preparedSource,
                            AudioGeneratorMP3*& preparedDecoder) {
        preparedSource = nullptr;
        preparedDecoder = nullptr;
        if (!audioReady || surahNumber < 1 ||
            surahNumber > TOTAL_SURAHS) return false;
        char url[180];
        if (!resolveQuranStreamUrl(surahNumber, url, sizeof(url))) {
            snprintf(url, sizeof(url),
                     "https://cdn.mp3quran.net/audio/maher-muaiqly/r1/%03u.mp3",
                     surahNumber);
            Serial.printf("[QURAN] Using validated direct stream fallback for Surah %u.\n",
                          surahNumber);
        }
        preparedSource = new (std::nothrow) HttpsAudioSource();
        preparedDecoder = new (std::nothrow) AudioGeneratorMP3();
        if (!preparedSource || !preparedDecoder) {
            Serial.println("[QURAN] Not enough memory to prepare playback.");
            delete preparedDecoder;
            delete preparedSource;
            preparedDecoder = nullptr;
            preparedSource = nullptr;
            return false;
        }
        if (!preparedSource->open(url)) {
            snprintf(url, sizeof(url),
                     "https://cdn.islamic.network/quran/audio/128/ar.alafasy/%u.mp3",
                     surahNumber);
            Serial.printf("[QURAN] Retrying Surah %u from alternate CDN.\n",
                          surahNumber);
        }
        if (!preparedSource->isOpen() && !preparedSource->open(url)) {
            Serial.printf("[QURAN] Could not start Surah %u stream.\n", surahNumber);
            delete preparedDecoder;
            preparedSource->close();
            delete preparedSource;
            preparedDecoder = nullptr;
            preparedSource = nullptr;
            return false;
        }
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
    portENTER_CRITICAL(&quranStateMux);
    ++quranRequestGeneration;
    quranQueuedStartFailed = false;
    quranPendingSurah = surahNumber;
    if (quranActive) quranStopRequested = true;
    portEXIT_CRITICAL(&quranStateMux);
    return true;
}

void stopQuran() {
    portENTER_CRITICAL(&quranStateMux);
    ++quranRequestGeneration;
    quranPendingSurah = -1;
    quranStopRequested = true;
    quranQueuedStartFailed = false;
    portEXIT_CRITICAL(&quranStateMux);
}

bool isQuranPlaying() {
    portENTER_CRITICAL(&quranStateMux);
    const bool active = quranActive && !quranStopRequested;
    portEXIT_CRITICAL(&quranStateMux);
    return active;
}

bool isQuranPending() {
    portENTER_CRITICAL(&quranStateMux);
    const bool pending = quranPendingSurah > 0 || quranStarting ||
                         quranPreparedSource != nullptr;
    portEXIT_CRITICAL(&quranStateMux);
    return pending;
}

bool takeQuranStartFailure() {
    portENTER_CRITICAL(&quranStateMux);
    const bool failed = quranQueuedStartFailed;
    quranQueuedStartFailed = false;
    portEXIT_CRITICAL(&quranStateMux);
    return failed;
}

void processQuranQueue() {
    uint8_t requestedSurah = 0;
    uint32_t requestGeneration = 0;
    portENTER_CRITICAL(&quranStateMux);
    if (quranPendingSurah > 0 && !playing && !quranActive &&
        !quranStarting && !quranPreparedSource && !quranStopRequested &&
        !touchBeepPending && !touchBeepActive &&
        !screenChangeBeepPending && !screenChangeBeepActive &&
        !pomodoroBeepPending && !pomodoroBeepActive) {
        requestedSurah = static_cast<uint8_t>(quranPendingSurah);
        quranPendingSurah = -1;
        quranStarting = true;
        requestGeneration = quranRequestGeneration;
    }
    portEXIT_CRITICAL(&quranStateMux);
    if (requestedSurah == 0) return;

    HttpsAudioSource* preparedSource = nullptr;
    AudioGeneratorMP3* preparedDecoder = nullptr;
    const bool prepared = prepareQuranStream(
        requestedSurah, preparedSource, preparedDecoder);

    bool accepted = false;
    bool reportFailure = false;
    portENTER_CRITICAL(&quranStateMux);
    quranStarting = false;
    if (prepared && requestGeneration == quranRequestGeneration &&
        !quranStopRequested && !quranActive) {
        quranPreparedSource = preparedSource;
        quranPreparedDecoder = preparedDecoder;
        quranPreparedGeneration = requestGeneration;
        accepted = true;
    } else if (!prepared &&
               requestGeneration == quranRequestGeneration &&
               !quranStopRequested) {
        quranQueuedStartFailed = true;
        reportFailure = true;
    }
    portEXIT_CRITICAL(&quranStateMux);

    if (accepted) {
        Serial.printf("[QURAN] Surah %u stream prepared for playback.\n",
                      requestedSurah);
    } else if (prepared) {
        delete preparedDecoder;
        preparedSource->close();
        delete preparedSource;
    } else if (reportFailure) {
        Serial.printf("[QURAN] Queued Surah %u could not be started.\n", requestedSurah);
    }
}

void update() {
    if (!audioReady || !out) return;

    HttpsAudioSource* discardedSource = nullptr;
    AudioGeneratorMP3* discardedDecoder = nullptr;
    portENTER_CRITICAL(&quranStateMux);
    const bool stopQuranNow = quranStopRequested;
    if (stopQuranNow) {
        discardedSource = quranPreparedSource;
        discardedDecoder = quranPreparedDecoder;
        quranPreparedSource = nullptr;
        quranPreparedDecoder = nullptr;
    }
    portEXIT_CRITICAL(&quranStateMux);

    if (stopQuranNow) {
        cleanupQuran();
        if (discardedDecoder) delete discardedDecoder;
        if (discardedSource) {
            discardedSource->close();
            delete discardedSource;
        }
        portENTER_CRITICAL(&quranStateMux);
        quranStopRequested = false;
        portEXIT_CRITICAL(&quranStateMux);
        if (playing || pomodoroBeepPending || screenChangeBeepPending ||
            touchBeepPending) {
            outputStarted = out->begin();
        }
    }

    HttpsAudioSource* startingSource = nullptr;
    AudioGeneratorMP3* startingDecoder = nullptr;
    uint32_t startingGeneration = 0;
    portENTER_CRITICAL(&quranStateMux);
    if (quranPreparedSource && quranPreparedDecoder &&
        !quranStopRequested && !quranActive && !playing &&
        !touchBeepPending && !touchBeepActive &&
        !screenChangeBeepPending && !screenChangeBeepActive &&
        !pomodoroBeepPending && !pomodoroBeepActive) {
        startingSource = quranPreparedSource;
        startingDecoder = quranPreparedDecoder;
        startingGeneration = quranPreparedGeneration;
        quranPreparedSource = nullptr;
        quranPreparedDecoder = nullptr;
        quranStarting = true;
    }
    portEXIT_CRITICAL(&quranStateMux);

    if (startingSource && startingDecoder) {
        const bool outputReady =
            outputStarted || (outputStarted = out->begin());
        const bool decoderStarted =
            outputReady && startingDecoder->begin(startingSource, out);
        bool accepted = false;
        if (decoderStarted) {
            quranSource = startingSource;
            quranDecoder = startingDecoder;
            quranDecoderStarted = true;
            out->SetGain(static_cast<float>(currentVolume) / 100.0f);
            outputStarted = true;
        }
        portENTER_CRITICAL(&quranStateMux);
        if (decoderStarted &&
            startingGeneration == quranRequestGeneration &&
            !quranStopRequested) {
            quranActive = true;
            accepted = true;
        } else if (!decoderStarted &&
                   startingGeneration == quranRequestGeneration &&
                   !quranStopRequested) {
            quranQueuedStartFailed = true;
        }
        quranStarting = false;
        portEXIT_CRITICAL(&quranStateMux);

        if (!accepted) {
            if (decoderStarted) startingDecoder->stop();
            if (!decoderStarted)
                Serial.println("[QURAN] Could not start prepared stream.");
            delete startingDecoder;
            startingSource->close();
            delete startingSource;
            if (decoderStarted) {
                quranDecoder = nullptr;
                quranSource = nullptr;
                quranDecoderStarted = false;
            }
            outputStarted = false;
        } else {
            Serial.println("[QURAN] Quran stream playback started.");
        }
    }

    if (touchBeepStopRequested) {
        touchBeepStopRequested = false;
        if (touchBeepActive) {
            touchBeepActive = false;
            out->SetGain(static_cast<float>(currentVolume) / 100.0f);
            if (playing && !startI2SOutput()) {
                Serial.println("[AZAN] Could not restart I2S after interrupting touch beep.");
            }
        }
    }

    portENTER_CRITICAL(&quranStateMux);
    const bool quranIsActive = quranActive;
    portEXIT_CRITICAL(&quranStateMux);
    if (quranIsActive && quranDecoder) {
        if (!quranDecoder->loop()) {
            Serial.println("[QURAN] Surah stream finished.");
            portENTER_CRITICAL(&quranStateMux);
            quranStopRequested = true;
            portEXIT_CRITICAL(&quranStateMux);
        }
        return;
    }

    portENTER_CRITICAL(&quranStateMux);
    const bool quranBlocksTouchBeep =
        quranActive || quranStarting || quranStopRequested;
    portEXIT_CRITICAL(&quranStateMux);
    if (touchBeepPending && !playing && !pomodoroBeepActive &&
        !screenChangeBeepActive && !touchBeepActive &&
        !quranBlocksTouchBeep) {
        touchBeepPending = false;
        if (!startI2SOutput()) {
            Serial.println("[AZAN] Could not start I2S for touch beep.");
        } else {
            out->SetGain(getTouchBeepGain());
            touchBeepActive = true;
            touchBeepStartedMs = millis();
            samplePhase = 0;
            Serial.println("[AZAN] Playing built-in touch beep through MAX98357A.");
        }
    } else if (screenChangeBeepPending && !playing && !pomodoroBeepActive &&
        !screenChangeBeepActive) {
        screenChangeBeepPending = false;
        if (!startI2SOutput()) {
            Serial.println("[AZAN] Could not start I2S for screen-change beep.");
        } else {
            out->SetGain(getScreenChangeBeepGain());
            screenChangeGainReduced = true;
            screenChangeBeepActive = true;
            screenChangeBeepStartedMs = millis();
            samplePhase = 0;
        }
    } else if (pomodoroBeepPending && !playing && !screenChangeBeepActive &&
               !touchBeepActive) {
        pomodoroBeepPending = false;
        if (!startI2SOutput()) {
            Serial.println("[AZAN] Could not start I2S for Pomodoro beep.");
        } else {
            pomodoroBeepActive = true;
            pomodoroBeepStartedMs = millis();
            samplePhase = 0;
        }
    }
    if (!playing && !pomodoroBeepActive && !screenChangeBeepActive &&
        !touchBeepActive &&
        beepEnvelope == 0) return;

    const unsigned long now = millis();
    if (pomodoroBeepActive &&
        now - pomodoroBeepStartedMs >= POMODORO_BEEP_LENGTH_MS) {
        pomodoroBeepActive = false;
    }
    if (screenChangeBeepActive &&
        now - screenChangeBeepStartedMs >= SCREEN_CHANGE_BEEP_LENGTH_MS) {
        screenChangeBeepActive = false;
    }
    if (touchBeepActive &&
        now - touchBeepStartedMs >= TOUCH_BEEP_LENGTH_MS) {
        touchBeepActive = false;
        out->SetGain(static_cast<float>(currentVolume) / 100.0f);
        Serial.println("[AZAN] Touch beep finished.");
    }
    if (screenChangeGainReduced && !screenChangeBeepActive &&
        !screenChangeBeepPending && !touchBeepActive &&
        !touchBeepPending) {
        out->SetGain(static_cast<float>(currentVolume) / 100.0f);
        screenChangeGainReduced = false;
    }
    const bool beepOn = playing
        ? isBeepOn(now - alarmStartedMs)
        : (pomodoroBeepActive || screenChangeBeepActive || touchBeepActive);
    const int16_t amplitude = screenChangeBeepActive
        ? SCREEN_CHANGE_BEEP_AMPLITUDE
        : (touchBeepActive ? TOUCH_BEEP_AMPLITUDE : BEEP_AMPLITUDE);
    const uint32_t phaseIncrement =
        (uint32_t)(((uint64_t)BEEP_FREQUENCY_HZ << 32) / AMP_SAMPLE_RATE);

    for (size_t i = 0; i < AUDIO_SAMPLES_PER_UPDATE; ++i) {
        int16_t value = 0;
        if (beepOn) {
            const int32_t scaledAmplitude =
                (static_cast<int32_t>(amplitude) * beepEnvelope) / 32767;
            const float phase = static_cast<float>(samplePhase) * PHASE_SCALE;
            value = static_cast<int16_t>(sinf(phase) * scaledAmplitude);
        }

        int16_t sample[2] = {value, value};
        if (!out->ConsumeSample(sample)) return;
        samplePhase += phaseIncrement;
        if (beepOn) {
            beepEnvelope = beepEnvelope > 32767 - BEEP_FADE_STEP
                ? 32767 : beepEnvelope + BEEP_FADE_STEP;
        } else {
            beepEnvelope = beepEnvelope > BEEP_FADE_STEP
                ? beepEnvelope - BEEP_FADE_STEP : 0;
        }
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
    touchBeepActive = false;
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
    portENTER_CRITICAL(&quranStateMux);
    const bool quranBusy = quranActive || quranStarting;
    portEXIT_CRITICAL(&quranStateMux);
    if (playing || quranBusy) return;
    touchBeepPending = true;
    Serial.println("[AZAN] Touch beep queued.");
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
