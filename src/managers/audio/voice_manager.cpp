#include "managers/audio/voice_manager.h"
#include "managers/audio/azan_manager.h"
#include <prayer_bot_config.h>
#include "network/https_client.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <driver/i2s.h>
#include <esp_heap_caps.h>
#include <freertos/FreeRTOS.h>
#include <mbedtls/base64.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

extern VoiceState g_voiceState;
extern volatile bool g_webAzanStartRequested;

namespace {
    constexpr size_t WAV_HEADER_BYTES = 44;
    constexpr size_t RECORDING_CAPACITY =
        MIC_SAMPLE_RATE * MIC_RECORD_SECONDS * sizeof(int16_t);
    constexpr size_t BASE64_INPUT_CHUNK_BYTES = 3072;
    constexpr size_t MAX_DASHBOARD_AUDIO_BYTES = 256 * 1024;
    constexpr size_t MAX_GEMINI_RESPONSE_BYTES = 400 * 1024;
    constexpr size_t MAX_INTERNAL_DASHBOARD_AUDIO_BYTES = 64 * 1024;
    constexpr size_t MIN_INTERNAL_AUDIO_HEADROOM_BYTES = 64 * 1024;
    constexpr size_t MAX_SPOKEN_TRANSCRIPT_LENGTH = 120;
    constexpr uint8_t GEMINI_AZAN_INTENT = TOTAL_SURAHS + 1;
    constexpr char GEMINI_MODEL[] = "gemini-3.8-flash";
    constexpr char GEMINI_LIVE_TRANSCRIPTION_MODEL[] =
        "models/gemini-3.5-transcribe-live";
    constexpr size_t MAX_LIVE_TOKEN_LENGTH = 256;

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
    esp_err_t micInitError = ESP_FAIL;
    volatile bool surahScreenActive = false;
    bool recognizedPlaybackPending = false;
    unsigned long recognizedResultStartedMs = 0;
    constexpr unsigned long RECOGNIZED_RESULT_DISPLAY_MS = 1200UL;
    volatile bool listenRequested = false;
    volatile bool recording = false;
    volatile bool recognitionPending = false;
    volatile bool recognitionInProgress = false;
    volatile bool cancelRequested = false;
    volatile bool playRequested = false;
    volatile bool skipNextAutoListen = false;
    volatile uint32_t screenGeneration = 0;
    uint32_t pendingGeneration = 0;
    volatile uint8_t requestedSurah = 0;
    uint8_t selectedSurah = 0;
    bool lastSelectionVoiceRecognized = false;
    uint8_t* recordingBuffer = nullptr;
    size_t capturedBytes = 0;
    unsigned long recordingStartedMs = 0;
    unsigned long lastMicReadErrorLogMs = 0;
    char statusText[40] = "Ready";
    portMUX_TYPE dashboardRecognitionMux = portMUX_INITIALIZER_UNLOCKED;
    char dashboardTranscript[MAX_SPOKEN_TRANSCRIPT_LENGTH + 1] = {};
    uint32_t dashboardRecognitionRequestId = 0;
    uint32_t nextDashboardRecognitionRequestId = 1;
    uint8_t dashboardRecognizedSurah = 0;
    bool dashboardRecognitionQueued = false;
    bool dashboardRecognitionRunning = false;
    bool dashboardRecognitionFinished = false;
    uint32_t dashboardLiveTokenRequestId = 0;
    uint32_t nextDashboardLiveTokenRequestId = 1;
    bool dashboardLiveTokenQueued = false;
    bool dashboardLiveTokenRunning = false;
    bool dashboardLiveTokenFinished = false;
    bool dashboardLiveTokenFailed = false;
    char dashboardLiveToken[MAX_LIVE_TOKEN_LENGTH] = {};
    uint32_t dashboardLiveTokenReadyMs = 0;
    uint32_t lastDashboardLiveTokenRequestMs = 0;
    bool dashboardLiveTokenRequestStarted = false;
    bool dashboardAudioQueued = false;
    uint8_t* dashboardAudioBuffer = nullptr;
    size_t dashboardAudioLength = 0;
    char dashboardAudioMimeType[32] = {};
    char dashboardAudioLanguage[8] = "auto";

    class AudioBufferOwner {
    public:
        explicit AudioBufferOwner(uint8_t* buffer) : buffer(buffer) {}
        // Audio buffers use heap_caps_malloc and are consumed by the request.
        ~AudioBufferOwner() { reset(); }
        AudioBufferOwner(const AudioBufferOwner&) = delete;
        AudioBufferOwner& operator=(const AudioBufferOwner&) = delete;

        void reset() {
            if (buffer) {
                heap_caps_free(buffer);
                buffer = nullptr;
            }
        }

    private:
        uint8_t* buffer;
    };

    uint8_t* allocateAudioBuffer(size_t length,
                                 size_t maxInternalAllocation) {
        uint8_t* buffer = static_cast<uint8_t*>(
            heap_caps_malloc(length, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
        if (buffer) return buffer;
        if (length > maxInternalAllocation) return nullptr;

        constexpr uint32_t internalMemoryCaps =
            MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT;
        const size_t freeBytes = heap_caps_get_free_size(internalMemoryCaps);
        const size_t largestBlock =
            heap_caps_get_largest_free_block(internalMemoryCaps);
        if (freeBytes < length || largestBlock < length ||
            freeBytes - length < MIN_INTERNAL_AUDIO_HEADROOM_BYTES) {
            return nullptr;
        }
        return static_cast<uint8_t*>(
            heap_caps_malloc(length, internalMemoryCaps));
    }

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

    class HttpBodyStream : public Stream {
    public:
        HttpBodyStream(WiFiClientSecure& client, bool chunked,
                       size_t contentLength, size_t maximumLength)
            : client(client), chunked(chunked), contentLength(contentLength),
              maximumLength(maximumLength) {}

        int available() override {
            if (bufferOffset >= bufferLength && !fillBuffer()) return 0;
            return static_cast<int>(bufferLength - bufferOffset);
        }

        int read() override {
            if (!available()) return -1;
            return buffer[bufferOffset++];
        }

        int peek() override {
            if (!available()) return -1;
            return buffer[bufferOffset];
        }

        void flush() override {}
        size_t write(uint8_t) override { return 0; }
        bool failed() const { return failedRead; }

    private:
        bool readNextChunk() {
            if (chunkStarted) {
                char terminator[2] = {};
                if (client.readBytes(reinterpret_cast<uint8_t*>(terminator),
                                     sizeof(terminator)) != sizeof(terminator) ||
                    terminator[0] != '\r' || terminator[1] != '\n') {
                    failedRead = true;
                    return false;
                }
            }

            String chunkLine = client.readStringUntil('\n');
            chunkLine.trim();
            const int extension = chunkLine.indexOf(';');
            if (extension >= 0) chunkLine.remove(extension);
            char* end = nullptr;
            const unsigned long chunkSize =
                strtoul(chunkLine.c_str(), &end, 16);
            if (chunkLine.isEmpty() || end == chunkLine.c_str() ||
                *end != '\0' ||
                chunkSize > maximumLength - bodyBytes) {
                failedRead = true;
                return false;
            }
            if (chunkSize == 0) {
                while (client.connected()) {
                    String trailer = client.readStringUntil('\n');
                    trailer.trim();
                    if (trailer.isEmpty()) break;
                }
                finished = true;
                return false;
            }

            chunkRemaining = static_cast<size_t>(chunkSize);
            chunkStarted = true;
            return true;
        }

        bool fillBuffer() {
            bufferOffset = 0;
            bufferLength = 0;
            if (finished || failedRead) return false;

            size_t wanted = sizeof(buffer);
            if (chunked) {
                if (chunkRemaining == 0 && !readNextChunk()) return false;
                wanted = min(wanted, chunkRemaining);
            } else {
                if (bodyBytes >= contentLength) {
                    finished = true;
                    return false;
                }
                wanted = min(wanted, contentLength - bodyBytes);
            }

            const size_t received = client.readBytes(buffer, wanted);
            if (received != wanted || received == 0) {
                failedRead = true;
                return false;
            }
            bufferLength = received;
            bodyBytes += received;
            if (chunked) chunkRemaining -= received;
            return true;
        }

        WiFiClientSecure& client;
        const bool chunked;
        const size_t contentLength;
        const size_t maximumLength;
        uint8_t buffer[512] = {};
        size_t bufferOffset = 0;
        size_t bufferLength = 0;
        size_t bodyBytes = 0;
        size_t chunkRemaining = 0;
        bool chunkStarted = false;
        bool finished = false;
        bool failedRead = false;
    };

    bool parseGeminiInteractionResponse(JsonDocument& reply,
                                        uint8_t& surahNumber) {
        const char* status = reply["status"] | "";
        if (strcmp(status, "completed") != 0) {
            Serial.printf("[VOICE] Gemini interaction did not complete (status: %s).\n",
                          status[0] ? status : "missing");
            return false;
        }

        String answer = reply["output_text"] | "";
        if (answer.length() > 16) {
            Serial.println("[VOICE] Gemini returned an oversized command.");
            return false;
        }
        if (answer.isEmpty()) {
            JsonArrayConst steps = reply["steps"].as<JsonArrayConst>();
            for (size_t i = steps.size(); i > 0; --i) {
                JsonObjectConst step = steps[i - 1].as<JsonObjectConst>();
                const char* type = step["type"] | "";
                if (strcmp(type, "user_input") == 0) break;
                if (strcmp(type, "model_output") != 0) continue;

                JsonArrayConst content =
                    step["content"].as<JsonArrayConst>();
                for (JsonObjectConst part : content) {
                    if (strcmp(part["type"] | "", "text") != 0) continue;
                    const char* text = part["text"] | "";
                    const size_t textLength = strlen(text);
                    if (answer.length() > 16 ||
                        textLength > 16 - answer.length() ||
                        !answer.concat(text)) {
                        Serial.println("[VOICE] Gemini returned an oversized command.");
                        return false;
                    }
                }
                if (!answer.isEmpty()) break;
            }
        }
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

    bool readGeminiInteractionResponse(WiFiClientSecure& client,
                                       uint8_t& surahNumber) {
        String statusLine = client.readStringUntil('\n');
        statusLine.trim();
        if (!statusLine.startsWith("HTTP/1.1 200") &&
            !statusLine.startsWith("HTTP/1.0 200") &&
            !statusLine.startsWith("HTTP/1.1 201") &&
            !statusLine.startsWith("HTTP/1.0 201")) {
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
            String normalizedHeader = header;
            normalizedHeader.toLowerCase();
            if (normalizedHeader.startsWith("content-length:"))
                responseLength = header.substring(15).toInt();
            else if (normalizedHeader.startsWith("transfer-encoding:") &&
                     normalizedHeader.indexOf("chunked") >= 0)
                chunked = true;
        }
        if ((!chunked && responseLength == 0) ||
            responseLength > MAX_GEMINI_RESPONSE_BYTES) {
            Serial.println("[VOICE] Gemini response size or encoding is unsupported.");
            client.stop();
            return false;
        }

        JsonDocument filter;
        filter["status"] = true;
        filter["output_text"] = true;
        filter["steps"][0]["type"] = true;
        filter["steps"][0]["content"][0]["type"] = true;
        filter["steps"][0]["content"][0]["text"] = true;
        JsonDocument response;
        HttpBodyStream body(client, chunked, responseLength,
                            MAX_GEMINI_RESPONSE_BYTES);
        const DeserializationError error = deserializeJson(
            response, body, DeserializationOption::Filter(filter),
            DeserializationOption::NestingLimit(12));
        if (error) {
            Serial.printf("[VOICE] Could not parse Gemini interaction response: %s.\n",
                          error.c_str());
            client.stop();
            return false;
        }
        if (body.failed()) {
            Serial.println("[VOICE] Gemini response stream was incomplete or exceeded its limit.");
            client.stop();
            return false;
        }
        client.stop();
        return parseGeminiInteractionResponse(response, surahNumber);
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
        request["model"] = GEMINI_MODEL;
        request["input"] = prompt;
        request["generation_config"]["max_output_tokens"] = 8;
        request["store"] = false;
        String body;
        if (serializeJson(request, body) == 0 || body.isEmpty()) {
            Serial.println("[VOICE] Could not serialize Gemini transcript request.");
            return false;
        }

        WiFiClientSecure client;
        configureHttpsClient(client);
        client.setTimeout(15000);
        if (!client.connect("generativelanguage.googleapis.com", 443, 15000)) {
            Serial.println("[VOICE] Gemini transcript connection failed.");
            return false;
        }

        client.printf("POST /v1beta/interactions HTTP/1.1\r\n"
                      "Host: generativelanguage.googleapis.com\r\n"
                      "X-goog-api-key: %s\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      apiKey, static_cast<unsigned>(body.length()));
        if (!writeAll(client, reinterpret_cast<const uint8_t*>(body.c_str()),
                      body.length())) {
            Serial.println("[VOICE] Could not send transcript to Gemini.");
            client.stop();
            return false;
        }
        return readGeminiInteractionResponse(client, surahNumber);
    }

    bool recognizeAudio(const char* apiKey, uint8_t* audioData,
                        size_t audioLength, const char* mimeType,
                        const char* language,
                        uint8_t& surahNumber) {
        AudioBufferOwner audioOwner(audioData);
        if (!apiKey || !apiKey[0] || !audioData || !audioLength ||
            !mimeType || !mimeType[0] ||
            WiFi.status() != WL_CONNECTED) {
            return false;
        }

        String prefix = "{\"model\":\"";
        prefix += GEMINI_MODEL;
        prefix +=
            "\",\"input\":["
            "{\"type\":\"text\",\"text\":\"Identify the requested Quran Surah in this short voice command. The speaker may use English, Bangla/Bengali, Arabic pronunciation, or phonetic transliteration, with brief silence or background noise. Focus on the spoken Surah name or number. Return only one integer: 1 through 114 for a requested Surah; 115 only for an explicit Azan-alarm request; otherwise 0.\"},"
            "{\"type\":\"audio\",\"mime_type\":\"";
        if (language && strcmp(language, "bn-BD") == 0) {
            prefix.replace(
                "The speaker may use English, Bangla/Bengali, Arabic pronunciation, or phonetic transliteration",
                "The selected language is Bangla (bn-BD); prioritize Bengali speech while accepting Arabic pronunciation or phonetic transliteration");
        } else if (language && strcmp(language, "en-US") == 0) {
            prefix.replace(
                "The speaker may use English, Bangla/Bengali, Arabic pronunciation, or phonetic transliteration",
                "The selected language is English (en-US); prioritize English speech while accepting Arabic pronunciation or phonetic transliteration");
        }
        prefix += mimeType;
        prefix +=
            "\",\"data\":\"";
        static const char suffix[] =
            "\"}],\"generation_config\":{\"max_output_tokens\":8},"
            "\"store\":false}";
        const size_t encodedLength = 4 * ((audioLength + 2) / 3);
        const size_t bodyLength = prefix.length() +
                                  encodedLength + (sizeof(suffix) - 1);

        WiFiClientSecure client;
        configureHttpsClient(client);
        client.setTimeout(15000);
        if (!client.connect("generativelanguage.googleapis.com", 443, 15000)) {
            Serial.println("[VOICE] Gemini connection failed.");
            return false;
        }

        client.printf("POST /v1beta/interactions HTTP/1.1\r\n"
                      "Host: generativelanguage.googleapis.com\r\n"
                      "X-goog-api-key: %s\r\n"
                      "Content-Type: application/json\r\n"
                      "Content-Length: %u\r\n"
                      "Connection: close\r\n\r\n",
                      apiKey, static_cast<unsigned>(bodyLength));

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

        audioOwner.reset();
        return readGeminiInteractionResponse(client, surahNumber);
    }

    bool createGeminiLiveToken(const char* apiKey, char* token,
                               size_t tokenCapacity) {
        if (!token || tokenCapacity == 0 || WiFi.status() != WL_CONNECTED) {
            return false;
        }
        if (!apiKey || !apiKey[0]) {
            Serial.println("[VOICE] Gemini API key is not configured for Live voice.");
            return false;
        }

        const time_t currentTime = time(nullptr);
        if (currentTime < 1700000000) {
            Serial.println("[VOICE] Cannot create a Live API token before time sync.");
            return false;
        }
        char sessionExpiry[25] = {};
        char tokenExpiry[25] = {};
        const time_t sessionExpiryTime = currentTime + 60;
        const time_t tokenExpiryTime = currentTime + 300;
        struct tm sessionExpiryUtc = {};
        struct tm tokenExpiryUtc = {};
        if (!gmtime_r(&sessionExpiryTime, &sessionExpiryUtc) ||
            !gmtime_r(&tokenExpiryTime, &tokenExpiryUtc) ||
            strftime(sessionExpiry, sizeof(sessionExpiry), "%Y-%m-%dT%H:%M:%SZ",
                     &sessionExpiryUtc) == 0 ||
            strftime(tokenExpiry, sizeof(tokenExpiry), "%Y-%m-%dT%H:%M:%SZ",
                     &tokenExpiryUtc) == 0) {
            Serial.println("[VOICE] Could not format Live API token expiry.");
            return false;
        }

        JsonDocument tokenRequest;
        tokenRequest["uses"] = 1;
        tokenRequest["newSessionExpireTime"] = sessionExpiry;
        tokenRequest["expireTime"] = tokenExpiry;
        JsonObject liveConstraints =
            tokenRequest["liveConnectConstraints"].to<JsonObject>();
        liveConstraints["model"] = GEMINI_LIVE_TRANSCRIPTION_MODEL;
        JsonObject liveConfig = liveConstraints["config"].to<JsonObject>();
        JsonArray responseModalities =
            liveConfig["responseModalities"].to<JsonArray>();
        responseModalities.add("TEXT");
        String requestBody;
        if (serializeJson(tokenRequest, requestBody) == 0 ||
            requestBody.isEmpty()) {
            Serial.println("[VOICE] Could not serialize Live API token request.");
            return false;
        }

        WiFiClientSecure tlsClient;
        configureHttpsClient(tlsClient);
        HTTPClient http;
        if (!http.begin(tlsClient,
                        "https://generativelanguage.googleapis.com/v1beta/auth_tokens")) {
            Serial.println("[VOICE] Could not connect to Gemini token service.");
            return false;
        }
        http.setTimeout(10000);
        http.addHeader("Content-Type", "application/json");
        http.addHeader("x-goog-api-key", apiKey);
        const int responseCode = http.POST(requestBody);
        requestBody.clear();
        if (responseCode != HTTP_CODE_OK) {
            Serial.printf("[VOICE] Live API token request failed with HTTP %d.\n",
                          responseCode);
            http.end();
            return false;
        }

        JsonDocument tokenFilter;
        tokenFilter["name"] = true;
        JsonDocument tokenReply;
        const DeserializationError parseError = deserializeJson(
            tokenReply, http.getStream(),
            DeserializationOption::Filter(tokenFilter),
            DeserializationOption::NestingLimit(4));
        http.end();
        const char* tokenName = tokenReply["name"] | "";
        if (parseError || !tokenName[0] ||
            strnlen(tokenName, tokenCapacity) >= tokenCapacity) {
            Serial.println("[VOICE] Gemini returned an invalid Live API token.");
            return false;
        }
        strlcpy(token, tokenName, tokenCapacity);
        return true;
    }

    bool hasSpeechEnergy(const uint8_t* pcm, size_t length, double& rms) {
        rms = 0;
        if (!pcm || length < sizeof(int16_t) * 10) return false;
        const int16_t* samples = reinterpret_cast<const int16_t*>(pcm);
        const size_t sampleCount = length / sizeof(int16_t);
        int64_t sampleSum = 0;
        for (size_t i = 0; i < sampleCount; ++i)
            sampleSum += samples[i];
        const double mean = static_cast<double>(sampleSum) / sampleCount;

        uint64_t sumSquares = 0;
        for (size_t i = 0; i < sampleCount; ++i) {
            const double sample = samples[i] - mean;
            sumSquares += static_cast<uint64_t>(sample * sample);
        }
        rms = sqrt(static_cast<double>(sumSquares) / sampleCount);
        return rms > 180.0;
    }

    void startPlayback(uint8_t surah, const char* fallbackReason,
                       VoiceState& state) {
        selectedSurah = surah;
        setStatus(fallbackReason ? fallbackReason : "Loading recitation");
        if (AzanManager::playQuranSurah(surah)) {
            if (!fallbackReason) setStatus("Playing recitation");
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
        .dma_buf_count = MIC_I2S_DMA_BUF_COUNT,
        .dma_buf_len = MIC_I2S_DMA_BUF_LEN,
        .use_apll = false
    };

    i2s_pin_config_t pinConfig = {
        .bck_io_num = PIN_MIC_SCK,
        .ws_io_num = PIN_MIC_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = PIN_MIC_SD
    };

    micInitError = i2s_driver_install(I2S_MIC_PORT, &i2sConfig, 0, nullptr);
    if (micInitError != ESP_OK) {
        Serial.printf("[VOICE] I2S microphone initialization failed: %s\n",
                      esp_err_to_name(micInitError));
        return;
    }
    micInitError = i2s_set_pin(I2S_MIC_PORT, &pinConfig);
    if (micInitError != ESP_OK) {
        i2s_driver_uninstall(I2S_MIC_PORT);
        Serial.printf("[VOICE] Could not configure microphone pins: %s\n",
                      esp_err_to_name(micInitError));
        return;
    }

    micInitialized = true;
    micInitError = ESP_OK;
    Serial.println("[VOICE] I2S microphone initialized.");
}

void startListening() {
    enterSurahScreen();
}

void enterSurahScreen() {
    surahScreenActive = true;
    recognizedPlaybackPending = false;
    lastSelectionVoiceRecognized = false;
    if (WiFi.status() != WL_CONNECTED) {
        listenRequested = false;
        cancelRequested = false;
        setStatus("Wi-Fi not connected");
        ::g_voiceState = VOICE_ERROR;
        return;
    }
    if (!micInitialized) init();
    if (!micInitialized) {
        listenRequested = false;
        cancelRequested = false;
        setStatus("Microphone unavailable");
        ::g_voiceState = VOICE_ERROR;
        Serial.printf("[VOICE] Cannot record: microphone is not initialized (%s).\n",
                      esp_err_to_name(micInitError));
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
    setStatus("Listening - say or select a Surah");
    ++screenGeneration;
    cancelRequested = false;
}

void leaveSurahScreen() {
    surahScreenActive = false;
    recognizedPlaybackPending = false;
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
        portENTER_CRITICAL(&dashboardRecognitionMux);
        const bool dashboardRecognitionBusy =
            dashboardRecognitionQueued || dashboardAudioQueued ||
            dashboardRecognitionRunning || dashboardLiveTokenQueued ||
            dashboardLiveTokenRunning;
        portEXIT_CRITICAL(&dashboardRecognitionMux);
        if (dashboardRecognitionBusy) return;

        listenRequested = false;
        recording = true;
        recordingStartedMs = millis();
        capturedBytes = 0;
        if (recordingBuffer) heap_caps_free(recordingBuffer);
        recordingBuffer = allocateAudioBuffer(
            RECORDING_CAPACITY + WAV_HEADER_BYTES,
            RECORDING_CAPACITY + WAV_HEADER_BYTES);
        if (recordingBuffer) {
            const esp_err_t clearResult = i2s_zero_dma_buffer(I2S_MIC_PORT);
            if (clearResult != ESP_OK) {
                Serial.printf("[VOICE] Could not clear I2S input buffer: %s\n",
                              esp_err_to_name(clearResult));
            }
        }
        if (!recordingBuffer) {
            recording = false;
            setStatus("Not enough memory for microphone");
            Serial.printf(
                "[VOICE] Audio allocation failed: %u bytes requested; "
                "internal free %u bytes, largest block %u bytes.\n",
                static_cast<unsigned>(RECORDING_CAPACITY + WAV_HEADER_BYTES),
                static_cast<unsigned>(heap_caps_get_free_size(
                    MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)),
                static_cast<unsigned>(heap_caps_get_largest_free_block(
                    MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)));
            state = VOICE_ERROR;
            return;
        }
        state = VOICE_LISTENING;
    }

    if (recording) {
        while (micInitialized && recordingBuffer &&
               capturedBytes < RECORDING_CAPACITY) {
            int32_t microphoneSamples[256];
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
            if (result != ESP_OK) {
                const unsigned long now = millis();
                if (lastMicReadErrorLogMs == 0 ||
                    now - lastMicReadErrorLogMs >= 3000UL) {
                    Serial.printf("[VOICE] I2S microphone read failed: %s\n",
                                  esp_err_to_name(result));
                    lastMicReadErrorLogMs = now;
                }
                break;
            }
            if (availableBytes < sizeof(int32_t)) break;

            const size_t sampleCount = min(
                availableBytes / sizeof(int32_t), requestedSamples);
            int16_t* outputSamples = reinterpret_cast<int16_t*>(
                recordingBuffer + capturedBytes);
            for (size_t i = 0; i < sampleCount; ++i)
                outputSamples[i] =
                    static_cast<int16_t>(microphoneSamples[i] >> 16);
            capturedBytes += sampleCount * sizeof(int16_t);
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
    uint32_t liveTokenRequestId = 0;
    bool processLiveToken = false;
    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardLiveTokenQueued && !dashboardRecognitionQueued &&
        !dashboardAudioQueued && !dashboardRecognitionRunning) {
        dashboardLiveTokenQueued = false;
        dashboardLiveTokenRunning = true;
        liveTokenRequestId = dashboardLiveTokenRequestId;
        processLiveToken = true;
    }
    portEXIT_CRITICAL(&dashboardRecognitionMux);

    if (processLiveToken) {
        char token[MAX_LIVE_TOKEN_LENGTH] = {};
        const bool succeeded = createGeminiLiveToken(
            settings.geminiApiKey, token, sizeof(token));
        portENTER_CRITICAL(&dashboardRecognitionMux);
        if (dashboardLiveTokenRequestId == liveTokenRequestId) {
            dashboardLiveTokenRunning = false;
            dashboardLiveTokenFinished = true;
            dashboardLiveTokenFailed = !succeeded;
            if (succeeded) {
                strlcpy(dashboardLiveToken, token, sizeof(dashboardLiveToken));
                dashboardLiveTokenReadyMs = millis();
            } else {
                dashboardLiveToken[0] = '\0';
                dashboardLiveTokenReadyMs = 0;
            }
        }
        portEXIT_CRITICAL(&dashboardRecognitionMux);
        memset(token, 0, sizeof(token));
    }

    char transcript[MAX_SPOKEN_TRANSCRIPT_LENGTH + 1] = {};
    uint32_t transcriptRequestId = 0;
    bool processTranscript = false;
    uint8_t* audioBuffer = nullptr;
    size_t audioLength = 0;
    char audioMimeType[sizeof(dashboardAudioMimeType)] = {};
    char audioLanguage[sizeof(dashboardAudioLanguage)] = "auto";
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
            strlcpy(audioLanguage, dashboardAudioLanguage,
                    sizeof(audioLanguage));
            dashboardAudioBuffer = nullptr;
            dashboardAudioLength = 0;
            dashboardAudioMimeType[0] = '\0';
            strlcpy(dashboardAudioLanguage, "auto",
                    sizeof(dashboardAudioLanguage));
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
        bool recognized = false;
        if (settings.geminiApiKey[0]) {
            if (processAudio) {
                uint8_t* ownedAudio = audioBuffer;
                audioBuffer = nullptr;
                recognized = recognizeAudio(settings.geminiApiKey, ownedAudio,
                                            audioLength, audioMimeType,
                                            audioLanguage,
                                            recognizedSurah);
            } else {
                recognized = recognizeSpokenTranscript(
                    settings.geminiApiKey, transcript, recognizedSurah);
            }
        }
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
                lastSelectionVoiceRecognized = false;
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
        recognizedPlaybackPending = false;
        if (WiFi.status() != WL_CONNECTED) {
            setStatus("Wi-Fi not connected");
            ::g_voiceState = VOICE_ERROR;
            return;
        }
        VoiceState state = VOICE_PROCESSING;
        startPlayback(requestedSurah, nullptr, state);
        ::g_voiceState = state;
    }
    if (recognizedPlaybackPending &&
        (!surahScreenActive || cancelRequested)) {
        recognizedPlaybackPending = false;
    } else if (recognizedPlaybackPending &&
               millis() - recognizedResultStartedMs >=
                   RECOGNIZED_RESULT_DISPLAY_MS) {
        recognizedPlaybackPending = false;
        VoiceState state = VOICE_PROCESSING;
        startPlayback(selectedSurah, nullptr, state);
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
        double speechRms = 0;
        const bool hasSpeech = hasSpeechEnergy(
            recordingBuffer, capturedBytes, speechRms);
        Serial.printf("[VOICE] Microphone captured %u bytes; RMS %.1f.\n",
                      static_cast<unsigned>(capturedBytes), speechRms);
        uint8_t recognizedSurah = 0;
        const char* apiKey = settings.geminiApiKey;
        if (hasSpeech && apiKey[0]) {
            if (capturedBytes & 1) --capturedBytes;
            memmove(recordingBuffer + WAV_HEADER_BYTES,
                    recordingBuffer, capturedBytes);
            makeWavHeader(recordingBuffer, capturedBytes);
            const size_t wavLength = capturedBytes + WAV_HEADER_BYTES;
            uint8_t* audioToRecognize = recordingBuffer;
            recordingBuffer = nullptr;
            if (recognizeAudio(apiKey, audioToRecognize,
                               wavLength, "audio/wav", "auto",
                               recognizedSurah) &&
                surahScreenActive && !cancelRequested &&
                requestGeneration == screenGeneration) {
                if (recognizedSurah >= 1 &&
                    recognizedSurah <= TOTAL_SURAHS) {
                    lastSelectionVoiceRecognized = true;
                    selectedSurah = recognizedSurah;
                    recognizedPlaybackPending = true;
                    recognizedResultStartedMs = millis();
                    state = VOICE_IDLE;
                    setStatus("Surah recognized");
                    Serial.printf("[VOICE] Gemini recognized Surah %u (%s).\n",
                                  recognizedSurah,
                                  SURAH_NAMES[recognizedSurah - 1]);
                } else if (recognizedSurah == GEMINI_AZAN_INTENT) {
                    g_webAzanStartRequested = true;
                    setStatus("Azan alarm requested");
                    state = VOICE_IDLE;
                    Serial.println("[VOICE] Gemini recognized an explicit Azan alarm request.");
                } else {
                    setStatus("Command was not a Surah");
                    state = VOICE_ERROR;
                    Serial.printf("[VOICE] Gemini returned unsupported voice command %u.\n",
                                  recognizedSurah);
                }
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
    recognizedPlaybackPending = false;
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

bool wasLastSelectionVoiceRecognized() {
    return lastSelectionVoiceRecognized;
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
        dashboardRecognitionRunning || dashboardLiveTokenQueued ||
        dashboardLiveTokenRunning) {
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
                                    const char* language,
                                    uint32_t& requestId) {
    if (WiFi.status() != WL_CONNECTED || !audio || audioLength == 0 ||
        audioLength > MAX_DASHBOARD_AUDIO_BYTES || !mimeType || !language ||
        recording || listenRequested || recognitionPending ||
        recognitionInProgress) return false;
    if (strcmp(mimeType, "audio/mp4") != 0 &&
        strcmp(mimeType, "audio/webm") != 0 &&
        strcmp(mimeType, "audio/wav") != 0 &&
        strcmp(mimeType, "audio/aac") != 0) return false;
    if (strcmp(language, "en-US") != 0 && strcmp(language, "bn-BD") != 0 &&
        strcmp(language, "auto") != 0) return false;

    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardRecognitionQueued || dashboardAudioQueued ||
        dashboardRecognitionRunning || dashboardLiveTokenQueued ||
        dashboardLiveTokenRunning) {
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
    strlcpy(dashboardAudioLanguage, language,
            sizeof(dashboardAudioLanguage));
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

bool queueDashboardLiveToken(uint32_t& requestId) {
    if (WiFi.status() != WL_CONNECTED || recording || listenRequested || recognitionPending ||
        recognitionInProgress) {
        return false;
    }

    const uint32_t now = millis();
    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardRecognitionQueued || dashboardAudioQueued ||
        dashboardRecognitionRunning || dashboardLiveTokenQueued ||
        dashboardLiveTokenRunning ||
        (dashboardLiveTokenRequestStarted &&
         now - lastDashboardLiveTokenRequestMs < 5000UL)) {
        portEXIT_CRITICAL(&dashboardRecognitionMux);
        return false;
    }

    uint32_t nextId = nextDashboardLiveTokenRequestId++;
    if (nextDashboardLiveTokenRequestId == 0)
        nextDashboardLiveTokenRequestId = 1;
    if (nextId == 0) {
        nextId = nextDashboardLiveTokenRequestId++;
        if (nextDashboardLiveTokenRequestId == 0)
            nextDashboardLiveTokenRequestId = 1;
    }
    dashboardLiveTokenRequestId = nextId;
    dashboardLiveTokenFinished = false;
    dashboardLiveTokenFailed = false;
    memset(dashboardLiveToken, 0, sizeof(dashboardLiveToken));
    dashboardLiveTokenReadyMs = 0;
    dashboardLiveTokenQueued = true;
    lastDashboardLiveTokenRequestMs = now;
    dashboardLiveTokenRequestStarted = true;
    requestId = nextId;
    portEXIT_CRITICAL(&dashboardRecognitionMux);
    return true;
}

const char* getDashboardLiveToken(uint32_t requestId, char* token,
                                  size_t tokenCapacity) {
    const char* status = "unknown";
    if (token && tokenCapacity) token[0] = '\0';
    portENTER_CRITICAL(&dashboardRecognitionMux);
    if (dashboardLiveTokenFinished && !dashboardLiveTokenFailed &&
        millis() - dashboardLiveTokenReadyMs >= 300000UL) {
        memset(dashboardLiveToken, 0, sizeof(dashboardLiveToken));
        dashboardLiveTokenFinished = false;
        dashboardLiveTokenReadyMs = 0;
    }
    if (requestId == dashboardLiveTokenRequestId && requestId != 0) {
        if (dashboardLiveTokenQueued) {
            status = "queued";
        } else if (dashboardLiveTokenRunning) {
            status = "processing";
        } else if (dashboardLiveTokenFinished && dashboardLiveTokenFailed) {
            status = "error";
        } else if (dashboardLiveTokenFinished) {
            if (token && tokenCapacity &&
                strnlen(dashboardLiveToken, tokenCapacity) < tokenCapacity) {
                strlcpy(token, dashboardLiveToken, tokenCapacity);
                memset(dashboardLiveToken, 0, sizeof(dashboardLiveToken));
                dashboardLiveTokenFinished = false;
                dashboardLiveTokenReadyMs = 0;
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
        recognizedPlaybackPending = false;
        selectedSurah = static_cast<uint8_t>(number);
        lastSelectionVoiceRecognized = false;
        skipNextAutoListen = !surahScreenActive;
        ::g_voiceState = VOICE_IDLE;
        setStatus("Selected from dashboard");
    }
}

void nextSurah() {
    recognizedPlaybackPending = false;
    selectedSurah = (selectedSurah % TOTAL_SURAHS) + 1;
    lastSelectionVoiceRecognized = false;
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
    recognizedPlaybackPending = false;
    selectedSurah = (selectedSurah <= 1) ? TOTAL_SURAHS : (selectedSurah - 1);
    lastSelectionVoiceRecognized = false;
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
    if (number < 1 || number > TOTAL_SURAHS) {
        setStatus(number < 1 ? "Select a Surah first" : "Invalid Surah number");
        return false;
    }
    recognizedPlaybackPending = false;
    selectedSurah = static_cast<uint8_t>(number);
    lastSelectionVoiceRecognized = false;
    skipNextAutoListen = !surahScreenActive;
    requestedSurah = selectedSurah;
    playRequested = true;
    setStatus("Preparing stream");
    return true;
}

} // namespace VoiceManager
