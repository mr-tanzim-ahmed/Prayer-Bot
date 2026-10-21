#include "voice_manager.h"
#include "azan_manager.h"
#include "config.h"
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <mbedtls/base64.h>
#include <math.h>
#include <stdlib.h>

extern VoiceState g_voiceState;

namespace {
    constexpr uint8_t DEFAULT_SURAH = 55;
    constexpr size_t WAV_HEADER_BYTES = 44;
    constexpr size_t RECORDING_CAPACITY =
        MIC_SAMPLE_RATE * MIC_RECORD_SECONDS * sizeof(int16_t);
    constexpr size_t MAX_GEMINI_RESPONSE_BYTES = 8192;
    constexpr size_t BASE64_INPUT_CHUNK_BYTES = 3072;
    constexpr size_t MAX_DASHBOARD_AUDIO_BYTES = 256 * 1024;
    constexpr size_t MAX_SPOKEN_TRANSCRIPT_LENGTH = 120;
    constexpr uint8_t GEMINI_AZAN_INTENT = TOTAL_SURAHS + 1;
    constexpr char GEMINI_MODEL[] = "gemini-2.5-flash";

    const char* const SURAH_NAMES[] = {
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

    bool micInitialized = false;
    volatile bool surahScreenActive = false;
    volatile bool listenRequested = false;
    volatile bool recording = false;
    volatile bool recognitionPending = false;
    volatile bool recognitionInProgress = false;
    volatile bool cancelRequested = false;
    volatile bool playRequested = false;
    volatile bool skipNextAutoListen = false;
    volatile uint32_t screenGeneration = 0;
    uint32_t pendingGeneration = 0;
    volatile uint8_t requestedSurah = DEFAULT_SURAH;
    uint8_t selectedSurah = DEFAULT_SURAH;
    uint8_t* recordingBuffer = nullptr;
    size_t capturedBytes = 0;
    unsigned long recordingStartedMs = 0;
    char statusText[40] = "Ready";
    portMUX_TYPE dashboardRecognitionMux = portMUX_INITIALIZER_UNLOCKED;
    char dashboardTranscript[MAX_SPOKEN_TRANSCRIPT_LENGTH + 1] = {};
    uint32_t dashboardRecognitionRequestId = 0;
    uint32_t nextDashboardRecognitionRequestId = 1;
    uint8_t dashboardRecognizedSurah = 0;
    bool dashboardRecognitionQueued = false;
    bool dashboardRecognitionRunning = false;
    bool dashboardRecognitionFinished = false;
    bool dashboardAudioQueued = false;
    uint8_t* dashboardAudioBuffer = nullptr;
    size_t dashboardAudioLength = 0;
    char dashboardAudioMimeType[32] = {};

    void setStatus(const char* status) {
        strlcpy(statusText, status, sizeof(statusText));
    }

    void freeRecording() {
        if (recordingBuffer) {
            heap_caps_free(recordingBuffer);
            recordingBuffer = nullptr;
        }
        capturedBytes = 0;
    }

    void writeLe16(uint8_t* buffer, size_t offset, uint16_t value) {
        buffer[offset] = static_cast<uint8_t>(value);
        buffer[offset + 1] = static_cast<uint8_t>(value >> 8);
    }

    void writeLe32(uint8_t* buffer, size_t offset, uint32_t value) {
        buffer[offset] = static_cast<uint8_t>(value);
        buffer[offset + 1] = static_cast<uint8_t>(value >> 8);
        buffer[offset + 2] = static_cast<uint8_t>(value >> 16);
        buffer[offset + 3] = static_cast<uint8_t>(value >> 24);
    }

    void makeWavHeader(uint8_t* buffer, size_t pcmBytes) {
        memcpy(buffer, "RIFF", 4);
        writeLe32(buffer, 4, static_cast<uint32_t>(36 + pcmBytes));
        memcpy(buffer + 8, "WAVEfmt ", 8);
        writeLe32(buffer, 16, 16);
        writeLe16(buffer, 20, 1);
        writeLe16(buffer, 22, 1);
        writeLe32(buffer, 24, MIC_SAMPLE_RATE);
        writeLe32(buffer, 28, MIC_SAMPLE_RATE * sizeof(int16_t));
        writeLe16(buffer, 32, sizeof(int16_t));
        writeLe16(buffer, 34, 16);
        memcpy(buffer + 36, "data", 4);
        writeLe32(buffer, 40, static_cast<uint32_t>(pcmBytes));
    }

    bool writeAll(WiFiClientSecure& client, const uint8_t* data, size_t length) {
        size_t written = 0;
        while (written < length) {
            const size_t count = client.write(data + written, length - written);
            if (count == 0) return false;
            written += count;
        }
        return true;
    }

    bool parseGeminiSurahResponse(const String& response,
                                 uint8_t& surahNumber) {
        JsonDocument reply;
        if (deserializeJson(reply, response)) {
            Serial.println("[VOICE] Gemini returned invalid JSON.");
            return false;
        }
        String answer =
            reply["candidates"][0]["content"]["parts"][0]["text"] | "";
        answer.trim();
        if (answer.isEmpty()) return false;

        uint16_t number = 0;
        for (size_t i = 0; i < answer.length(); ++i) {
            const char digit = answer[i];
            if (digit < '0' || digit > '9') return false;
            number = number * 10 + static_cast<uint8_t>(digit - '0');
            if (number > GEMINI_AZAN_INTENT) return false;
        }
        if (number < 1) return false;
        surahNumber = static_cast<uint8_t>(number);
        return true;
    }

    bool readGeminiSurahResponse(WiFiClientSecure& client,
                                 uint8_t& surahNumber) {
        String statusLine = client.readStringUntil('\n');
        statusLine.trim();
        if (!statusLine.startsWith("HTTP/1.1 200") &&
            !statusLine.startsWith("HTTP/1.0 200")) {
            Serial.printf("[VOICE] Gemini returned %s\n", statusLine.c_str());
            client.stop();
            return false;
        }

        size_t responseLength = 0;
        bool chunked = false;
        while (client.connected()) {
            String header = client.readStringUntil('\n');
            header.trim();
            if (header.isEmpty()) break;
            if (header.startsWith("Content-Length:"))
                responseLength = header.substring(15).toInt();
            else if (header.equalsIgnoreCase("Transfer-Encoding: chunked"))
                chunked = true;
        }
        if (chunked || responseLength == 0 ||
            responseLength > MAX_GEMINI_RESPONSE_BYTES) {
            Serial.println("[VOICE] Gemini response size or encoding is unsupported.");
            client.stop();
            return false;
        }

        String response;
        if (!response.reserve(responseLength)) {
            Serial.println("[VOICE] Not enough memory to read Gemini response.");
            client.stop();
            return false;
        }
        char responseChunk[512];
        size_t received = 0;
        while (received < responseLength) {
            const size_t wanted = min(sizeof(responseChunk),
                                      responseLength - received);
            const size_t count = client.readBytes(
                reinterpret_cast<uint8_t*>(responseChunk), wanted);
            if (!count || !response.concat(responseChunk, count)) {
                Serial.println("[VOICE] Gemini response ended unexpectedly.");
                client.stop();
                return false;
            }
            received += count;
        }
        client.stop();
        return parseGeminiSurahResponse(response, surahNumber);
    }

    bool recognizeSpokenTranscript(const char* apiKey, const char* transcript,
                                   uint8_t& surahNumber) {
        if (!apiKey || !apiKey[0] || !transcript || !transcript[0] ||
            WiFi.status() != WL_CONNECTED) {
            return false;
        }

        String prompt =
            "Classify this short voice command for a Quran and prayer device. "
            "The transcript may be English, Bangla/Bengali, or phonetic "
            "transliteration: ";
        prompt += transcript;
        prompt +=
            ". Return only one integer: 1 through 114 when the user asks to "
            "find or play that Quran Surah; 115 only when the user explicitly "
            "asks to play/trigger the device's Azan alarm; otherwise 0. "
            "Do not infer an Azan request from a Surah request.";
        JsonDocument request;
        request["contents"][0]["parts"][0]["text"] = prompt;
        request["generationConfig"]["temperature"] = 0;
        String body;
        if (serializeJson(request, body) == 0 || body.isEmpty()) {
            Serial.println("[VOICE] Could not serialize Gemini transcript request.");
            return false;
        }

        WiFiClientSecure client;
        client.setInsecure();
        client.setTimeout(5000);
        if (!client.connect("generativelanguage.googleapis.com", 443, 15000)) {
            Serial.println("[VOICE] Gemini transcript connection failed.");
            return false;
        }

        client.printf("POST /v1beta/models/%s:generateContent HTTP/1.1\r\n"
                      "Host: generativelanguage.googleapis.com\r\n"
                      "X-goog-api-key: %s\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      GEMINI_MODEL, apiKey, static_cast<unsigned>(body.length()));
        if (!writeAll(client, reinterpret_cast<const uint8_t*>(body.c_str()),
                      body.length())) {
            Serial.println("[VOICE] Could not send transcript to Gemini.");
            client.stop();
            return false;
        }
        return readGeminiSurahResponse(client, surahNumber);
    }

    bool recognizeAudio(const char* apiKey, const uint8_t* audioData,
                        size_t audioLength, const char* mimeType,
                        uint8_t& surahNumber) {
        if (!apiKey || !apiKey[0] || !audioData || !audioLength ||
            !mimeType || !mimeType[0] ||
            WiFi.status() != WL_CONNECTED) {
            return false;
        }

        String prefix =
            "{\"contents\":[{\"parts\":["
            "{\"text\":\"Identify this short voice command for a Quran and prayer device. The speaker may use English, Bangla/Bengali, or common transliteration. Return only one integer: 1 through 114 for a requested Quran Surah; 115 only for an explicit request to play the Azan alarm; otherwise 0.\"},"
            "{\"inlineData\":{\"mimeType\":\"";
        prefix += mimeType;
        prefix += "\",\"data\":\"";
        static const char suffix[] =
            "\"}}]}],\"generationConfig\":{\"temperature\":0}}";
        const size_t encodedLength = 4 * ((audioLength + 2) / 3);
        const size_t bodyLength = prefix.length() +
                                  encodedLength + (sizeof(suffix) - 1);

        WiFiClientSecure client;
        client.setInsecure();
        client.setTimeout(3000);
        if (!client.connect("generativelanguage.googleapis.com", 443, 15000)) {
            Serial.println("[VOICE] Gemini connection failed.");
            return false;
        }

        client.printf("POST /v1beta/models/%s:generateContent HTTP/1.1\r\n"
                      "Host: generativelanguage.googleapis.com\r\n"
                      "X-goog-api-key: %s\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      GEMINI_MODEL, apiKey, static_cast<unsigned>(bodyLength));

        if (!writeAll(client, reinterpret_cast<const uint8_t*>(prefix.c_str()),
                      prefix.length())) {
            Serial.println("[VOICE] Could not send Gemini request prefix.");
            client.stop();
            return false;
        }

        uint8_t encoded[4097];
        size_t offset = 0;
        while (offset < audioLength) {
            size_t inputLength = audioLength - offset;
            if (inputLength > BASE64_INPUT_CHUNK_BYTES)
                inputLength = BASE64_INPUT_CHUNK_BYTES;
            if (offset + inputLength < audioLength)
                inputLength -= inputLength % 3;

            size_t outputLength = 0;
            const int result = mbedtls_base64_encode(
                encoded, sizeof(encoded), &outputLength,
                audioData + offset, inputLength);
            if (result != 0 ||
                !writeAll(client, encoded, outputLength)) {
                Serial.println("[VOICE] Could not stream microphone audio to Gemini.");
                client.stop();
                return false;
            }
            offset += inputLength;
        }

        if (!writeAll(client, reinterpret_cast<const uint8_t*>(suffix),
                      sizeof(suffix) - 1)) {
            Serial.println("[VOICE] Could not complete Gemini request.");
            client.stop();
            return false;
        }

        return readGeminiSurahResponse(client, surahNumber);
    }

    bool hasSpeechEnergy(const uint8_t* pcm, size_t length) {
        if (!pcm || length < sizeof(int16_t) * 10) return false;
        const int16_t* samples = reinterpret_cast<const int16_t*>(pcm);
        const size_t sampleCount = length / sizeof(int16_t);
        uint64_t sumSquares = 0;
        for (size_t i = 0; i < sampleCount; ++i) {
            const int32_t sample = samples[i];
            sumSquares += static_cast<uint64_t>(sample * sample);
        }
        const double rms = sqrt(static_cast<double>(sumSquares) / sampleCount);
        return rms > 180.0;
    }

    void startPlayback(uint8_t surah, const char* fallbackReason,
                       VoiceState& state) {
        selectedSurah = surah;
        setStatus(fallbackReason ? fallbackReason : "Loading recitation");
        if (AzanManager::playQuranSurah(surah)) {
            if (!fallbackReason) setStatus("Playing mp3quran.net");
            state = VOICE_PLAYING;
        } else {
            setStatus("Audio stream unavailable");
            state = VOICE_ERROR;
        }
    }
}

namespace VoiceManager {

void init() {
    if (micInitialized) return;

    i2s_config_t i2sConfig = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = MIC_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
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

    if (i2s_driver_install(I2S_MIC_PORT, &i2sConfig, 0, nullptr) != ESP_OK) {
        Serial.println("[VOICE] I2S microphone initialization failed.");
        return;
    }
    if (i2s_set_pin(I2S_MIC_PORT, &pinConfig) != ESP_OK) {
        i2s_driver_uninstall(I2S_MIC_PORT);
        Serial.println("[VOICE] Could not configure microphone pins.");
        return;
    }

    micInitialized = true;
    Serial.println("[VOICE] I2S microphone initialized.");
}

void startListening() {
    enterSurahScreen();
}

void enterSurahScreen() {
    surahScreenActive = true;
    if (WiFi.status() != WL_CONNECTED) {
        listenRequested = false;
        cancelRequested = false;
        setStatus("Wi-Fi not connected");
        ::g_voiceState = VOICE_ERROR;
        return;
    }
    if (skipNextAutoListen) {
        skipNextAutoListen = false;
        cancelRequested = false;
        return;
    }
    listenRequested = true;
    recognitionPending = false;
    cancelRequested = false;
    selectedSurah = DEFAULT_SURAH;
    setStatus("Listening - say Surah");
    ++screenGeneration;
    cancelRequested = false;
}

void leaveSurahScreen() {
    surahScreenActive = false;
    ++screenGeneration;
    listenRequested = false;
    playRequested = false;
    recording = false;
    recognitionPending = false;
    cancelRequested = true;
    if (!recognitionInProgress) freeRecording();
    AzanManager::stopQuran();
    setStatus("Playback stopped");
}

void update(VoiceState& state) {
    if (listenRequested && !recognitionInProgress) {
        listenRequested = false;
        recording = true;
        recordingStartedMs = millis();
        capturedBytes = 0;
        if (recordingBuffer) heap_caps_free(recordingBuffer);
        recordingBuffer = static_cast<uint8_t*>(
            heap_caps_malloc(RECORDING_CAPACITY + WAV_HEADER_BYTES,
                             MALLOC_CAP_8BIT));
        if (recordingBuffer && micInitialized) {
            i2s_zero_dma_buffer(I2S_MIC_PORT);
        }
        if (!recordingBuffer) {
            recording = false;
            setStatus("Not enough memory for microphone");
            state = VOICE_ERROR;
            return;
        }
        state = VOICE_LISTENING;
    }

    if (recording) {
        if (micInitialized && recordingBuffer &&
            capturedBytes < RECORDING_CAPACITY) {
            int32_t microphoneSamples[800];
            size_t availableBytes = 0;
            const size_t remaining = RECORDING_CAPACITY - capturedBytes;
            const size_t requestedSamples = min(
                sizeof(microphoneSamples) / sizeof(microphoneSamples[0]),
                remaining / sizeof(int16_t));
            const size_t chunkBytes =
                requestedSamples * sizeof(microphoneSamples[0]);
            const esp_err_t result = i2s_read(
                I2S_MIC_PORT, microphoneSamples, chunkBytes,
                &availableBytes, 0);
            if (result == ESP_OK && availableBytes >= sizeof(int32_t)) {
                const size_t sampleCount = min(
                    availableBytes / sizeof(int32_t), requestedSamples);
                int16_t* outputSamples = reinterpret_cast<int16_t*>(
                    recordingBuffer + capturedBytes);
                for (size_t i = 0; i < sampleCount; ++i)
                    outputSamples[i] =
                        static_cast<int16_t>(microphoneSamples[i] >> 16);
                capturedBytes += sampleCount * sizeof(int16_t);
            }
        }

        if (millis() - recordingStartedMs >= MIC_RECORD_SECONDS * 1000UL) {
            recording = false;
            recognitionPending = true;
            pendingGeneration = screenGeneration;
            state = VOICE_PROCESSING;
            setStatus("Recognizing Surah name");
        }
        return;
    }

    if (state == VOICE_PLAYING && !AzanManager::isQuranPlaying()) {
        if (AzanManager::isQuranPending()) return;
        state = VOICE_IDLE;
        setStatus("Recitation finished");
    }
}

void processPending(const Settings& settings) {
    AzanManager::processQuranQueue();
    char transcript[MAX_SPOKEN_TRANSCRIPT_LENGTH + 1] = {};
    uint32_t transcriptRequestId = 0;
    bool processTranscript = false;
    uint8_t* audioBuffer = nullptr;
    size_t audioLength = 0;
    char audioMimeType[sizeof(dashboardAudioMimeType)] = {};
    bool processAudio = false;
    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardRecognitionQueued || dashboardAudioQueued) {
        dashboardRecognitionQueued = false;
        dashboardAudioQueued = false;
        dashboardRecognitionRunning = true;
        transcriptRequestId = dashboardRecognitionRequestId;
        if (dashboardAudioBuffer) {
            audioBuffer = dashboardAudioBuffer;
            audioLength = dashboardAudioLength;
            strlcpy(audioMimeType, dashboardAudioMimeType, sizeof(audioMimeType));
            dashboardAudioBuffer = nullptr;
            dashboardAudioLength = 0;
            dashboardAudioMimeType[0] = '\0';
            processAudio = true;
        } else {
            strlcpy(transcript, dashboardTranscript, sizeof(transcript));
            processTranscript = true;
        }
    }
    portEXIT_CRITICAL(&dashboardRecognitionMux);

    if (processTranscript || processAudio) {
        ::g_voiceState = VOICE_PROCESSING;
        uint8_t recognizedSurah = 0;
        const bool recognized = settings.geminiApiKey[0] &&
            (processAudio
                ? recognizeAudio(settings.geminiApiKey, audioBuffer, audioLength,
                                 audioMimeType, recognizedSurah)
                : recognizeSpokenTranscript(settings.geminiApiKey, transcript,
                                            recognizedSurah));
        if (audioBuffer) heap_caps_free(audioBuffer);
        portENTER_CRITICAL(&dashboardRecognitionMux);
        if (dashboardRecognitionRequestId == transcriptRequestId) {
            dashboardRecognitionRunning = false;
            dashboardRecognitionFinished = true;
            dashboardRecognizedSurah = recognized ? recognizedSurah : 0;
        }
        portEXIT_CRITICAL(&dashboardRecognitionMux);
        if (recognized) {
            if (recognizedSurah <= TOTAL_SURAHS) {
                selectedSurah = recognizedSurah;
                skipNextAutoListen = true;
                setStatus("Dashboard voice recognized");
                Serial.printf("[VOICE] Dashboard transcript recognized Surah %u (%s).\n",
                              recognizedSurah, SURAH_NAMES[recognizedSurah - 1]);
            } else {
                setStatus("Dashboard Azan alarm request");
                Serial.println("[VOICE] Dashboard transcript requested the Azan alarm.");
            }
            ::g_voiceState = VOICE_IDLE;
        } else {
            setStatus(settings.geminiApiKey[0]
                ? "Gemini could not identify Surah"
                : "Gemini key not configured");
            ::g_voiceState = VOICE_ERROR;
            Serial.println("[VOICE] Gemini could not identify the dashboard transcript.");
        }
    }

    if (playRequested) {
        playRequested = false;
        if (WiFi.status() != WL_CONNECTED) {
            setStatus("Wi-Fi not connected");
            ::g_voiceState = VOICE_ERROR;
            return;
        }
        VoiceState state = VOICE_PROCESSING;
        startPlayback(requestedSurah, nullptr, state);
        ::g_voiceState = state;
    }
    if (AzanManager::takeQuranStartFailure()) {
        setStatus("Audio stream unavailable");
        ::g_voiceState = VOICE_ERROR;
    }
    if (!recognitionPending || recognitionInProgress) return;
    recognitionPending = false;
    recognitionInProgress = true;
    const uint32_t requestGeneration = pendingGeneration;

    VoiceState state = VOICE_PROCESSING;
    bool started = false;
    if (surahScreenActive && !cancelRequested &&
        requestGeneration == screenGeneration) {
        if (WiFi.status() != WL_CONNECTED) {
            setStatus("Wi-Fi not connected");
            ::g_voiceState = VOICE_ERROR;
            freeRecording();
            recognitionInProgress = false;
            return;
        }
        const bool hasSpeech = hasSpeechEnergy(recordingBuffer, capturedBytes);
        uint8_t recognizedSurah = 0;
        const char* apiKey = settings.geminiApiKey;
        if (hasSpeech && apiKey[0]) {
            if (capturedBytes & 1) --capturedBytes;
            memmove(recordingBuffer + WAV_HEADER_BYTES,
                    recordingBuffer, capturedBytes);
            makeWavHeader(recordingBuffer, capturedBytes);
            const size_t wavLength = capturedBytes + WAV_HEADER_BYTES;
            if (recognizeAudio(apiKey, recordingBuffer,
                               wavLength, "audio/wav", recognizedSurah) &&
                surahScreenActive && !cancelRequested &&
                requestGeneration == screenGeneration) {
                Serial.printf("[VOICE] Recognized Surah %u (%s).\n",
                              recognizedSurah, SURAH_NAMES[recognizedSurah - 1]);
                startPlayback(recognizedSurah, nullptr, state);
            } else {
                setStatus("Gemini recognition failed");
                state = VOICE_ERROR;
            }
        } else if (!micInitialized || !recordingBuffer) {
            setStatus("Microphone unavailable");
            state = VOICE_ERROR;
        } else if (!apiKey[0] && hasSpeech) {
            setStatus("Gemini key not configured");
            state = VOICE_ERROR;
        } else {
            setStatus("No speech detected; try again");
            state = VOICE_ERROR;
        }
        started = surahScreenActive && !cancelRequested &&
                  requestGeneration == screenGeneration;
        if (!started) AzanManager::stopQuran();
    }

    freeRecording();
    recognitionInProgress = false;
    if (started) ::g_voiceState = state;
}

void stopPlayback() {
    playRequested = false;
    AzanManager::stopQuran();
    setStatus("Playback stopped");
}

uint8_t getRecognizedSurah() {
    return selectedSurah;
}

const char* getSurahName(uint8_t number) {
    if (number >= 1 && number <= TOTAL_SURAHS)
        return SURAH_NAMES[number - 1];
    return "Unknown";
}

uint8_t getPlayingSurah() {
    return selectedSurah;
}

const char* getStatusText() {
    return statusText;
}

bool isListening() {
    return recording;
}

bool isProcessing() {
    return recognitionPending || recognitionInProgress;
}

bool isPlaying() {
    return AzanManager::isQuranPlaying();
}

bool hasError() {
    return ::g_voiceState == VOICE_ERROR;
}

bool queueDashboardRecognition(const char* transcript, uint32_t& requestId) {
    if (WiFi.status() != WL_CONNECTED || !transcript) return false;
    const size_t length = strnlen(transcript, MAX_SPOKEN_TRANSCRIPT_LENGTH + 1);
    if (length == 0 || length > MAX_SPOKEN_TRANSCRIPT_LENGTH) return false;

    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardRecognitionQueued || dashboardAudioQueued ||
        dashboardRecognitionRunning) {
        portEXIT_CRITICAL(&dashboardRecognitionMux);
        return false;
    }
    uint32_t nextId = nextDashboardRecognitionRequestId++;
    if (nextId == 0) nextId = nextDashboardRecognitionRequestId++;
    dashboardRecognitionRequestId = nextId;
    dashboardRecognizedSurah = 0;
    dashboardRecognitionFinished = false;
    strlcpy(dashboardTranscript, transcript, sizeof(dashboardTranscript));
    dashboardRecognitionQueued = true;
    requestId = nextId;
    portEXIT_CRITICAL(&dashboardRecognitionMux);
    return true;
}

bool queueDashboardAudioRecognition(uint8_t* audio, size_t audioLength,
                                    const char* mimeType,
                                    uint32_t& requestId) {
    if (WiFi.status() != WL_CONNECTED || !audio || audioLength == 0 ||
        audioLength > MAX_DASHBOARD_AUDIO_BYTES || !mimeType) return false;
    if (strcmp(mimeType, "audio/mp4") != 0 &&
        strcmp(mimeType, "audio/webm") != 0 &&
        strcmp(mimeType, "audio/wav") != 0 &&
        strcmp(mimeType, "audio/aac") != 0) return false;

    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardRecognitionQueued || dashboardAudioQueued ||
        dashboardRecognitionRunning) {
        portEXIT_CRITICAL(&dashboardRecognitionMux);
        return false;
    }
    uint32_t nextId = nextDashboardRecognitionRequestId++;
    if (nextId == 0) nextId = nextDashboardRecognitionRequestId++;
    dashboardRecognitionRequestId = nextId;
    dashboardRecognizedSurah = 0;
    dashboardRecognitionFinished = false;
    dashboardAudioBuffer = audio;
    dashboardAudioLength = audioLength;
    strlcpy(dashboardAudioMimeType, mimeType, sizeof(dashboardAudioMimeType));
    dashboardAudioQueued = true;
    requestId = nextId;
    portEXIT_CRITICAL(&dashboardRecognitionMux);
    return true;
}

const char* getDashboardRecognition(uint32_t requestId,
                                    uint8_t& surahNumber) {
    const char* status = "unknown";
    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (requestId == dashboardRecognitionRequestId && requestId != 0) {
        if (dashboardRecognitionQueued || dashboardAudioQueued) {
            status = "queued";
        } else if (dashboardRecognitionRunning) {
            status = "processing";
        } else if (dashboardRecognitionFinished) {
            if (dashboardRecognizedSurah != 0) {
                surahNumber = dashboardRecognizedSurah;
                status = "done";
            } else {
                status = "error";
            }
        }
    }
    portEXIT_CRITICAL(&dashboardRecognitionMux);
    return status;
}

void selectSurah(int number) {
    if (number >= 1 && number <= TOTAL_SURAHS) {
        selectedSurah = static_cast<uint8_t>(number);
        skipNextAutoListen = !surahScreenActive;
        ::g_voiceState = VOICE_IDLE;
        setStatus("Selected from dashboard");
    }
}

void nextSurah() {
    selectedSurah = (selectedSurah % TOTAL_SURAHS) + 1;
    skipNextAutoListen = !surahScreenActive;
    if (AzanManager::isQuranPlaying()) {
        playSurah(selectedSurah);
    } else {
        char buf[36];
        snprintf(buf, sizeof(buf), "Selected: %s", getSurahName(selectedSurah));
        setStatus(buf);
    }
}

void prevSurah() {
    selectedSurah = (selectedSurah <= 1) ? TOTAL_SURAHS : (selectedSurah - 1);
    skipNextAutoListen = !surahScreenActive;
    if (AzanManager::isQuranPlaying()) {
        playSurah(selectedSurah);
    } else {
        char buf[36];
        snprintf(buf, sizeof(buf), "Selected: %s", getSurahName(selectedSurah));
        setStatus(buf);
    }
}

bool playSurah(int number) {
    if (WiFi.status() != WL_CONNECTED) {
        setStatus("Wi-Fi not connected");
        ::g_voiceState = VOICE_ERROR;
        return false;
    }
    if (number < 1 || number > TOTAL_SURAHS) number = selectedSurah;
    selectedSurah = static_cast<uint8_t>(number);
    skipNextAutoListen = !surahScreenActive;
    requestedSurah = selectedSurah;
    playRequested = true;
    setStatus("Preparing stream");
    return true;
}

} // namespace VoiceManager
