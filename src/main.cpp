#include <Arduino.h>
#include <WiFi.h>
#include "config.h"
#include "types.h"

// Module headers
#include "wifi_manager.h"
#include "display_manager.h"
#include "prayer_manager.h"
#include "weather_manager.h"
#include "azan_manager.h"
#include "shake_detector.h"
#include "button_handler.h"
#include "pomodoro_manager.h"
#include "calendar_manager.h"
#include "voice_manager.h"
#include "storage_manager.h"
#include "web_server_manager.h"
#include "screen_router.h"
#include "logger.h"
#include "cache_manager.h"
#include "screens/dhikr_screen.h"
#include "pomodoro_stats.h"

// ============================================================
//  PRAYER-BOT MAIN
//  Application entry point. Initializes all modules and runs
//  the main loop via FreeRTOS tasks.
// ============================================================

// --- Global shared state ---
Settings        g_settings;
WeatherData     g_weather;
DailyPrayers    g_todayPrayers;
NextPrayerInfo  g_nextPrayer;
ProhibitedTimes g_prohibited;
HijriDate       g_hijriDate;
PomodoroState   g_pomodoro;
IslamicEvent    g_todayEvent;

SystemScreen    g_currentScreen = SYS_SCREEN_PRAYER_FOCUS;
unsigned long   g_screenStartMs = 0;
volatile AzanState g_azanState  = AZAN_IDLE;
volatile bool   g_azanStopRequested = false;
volatile bool   g_locationRefreshRequested = false;
volatile bool   g_weatherRefreshRequested = false;
volatile bool   g_ntpSyncRequested = false;
volatile int8_t g_webScreenRequest = -1;
volatile uint8_t g_webPomodoroRequest = 0;
volatile uint8_t g_webDhikrRequest = 0;
volatile int16_t g_webVolumeRequest = -1;
VoiceState      g_voiceState    = VOICE_IDLE;

bool            g_wifiConnected  = false;
bool            g_timesynced     = false;
bool            g_weatherReady   = false;
bool            g_prayerReady    = false;

// --- FreeRTOS task handles ---
TaskHandle_t taskDisplayHandle   = NULL;
TaskHandle_t taskSchedulerHandle = NULL;
TaskHandle_t taskAudioHandle     = NULL;
TaskHandle_t taskNetworkHandle   = NULL;
TaskHandle_t taskWebServerHandle = NULL;

// --- Task functions ---
void taskDisplay(void* param);
void taskScheduler(void* param);
void taskNetwork(void* param);
void taskAudio(void* param);

namespace {
    void resumePomodoroAfterAzan() {
        if (g_pomodoro.pausedByAzan) {
            PomodoroManager::resumeFromAzan(g_pomodoro);
        }
    }
}

// ============================================================
//  SETUP
// ============================================================

void setup() {
    Serial.begin(115200);
    delay(300);

    Serial.println();
    Serial.println("========================================");
    Serial.println("   PRAYER-BOT");
    Serial.println("   Islamic Personal Assistant");
    Serial.println("========================================");

    // 1. Initialize storage and load settings
    Logger::init(LOG_DEBUG);
    StorageManager::init();
    CacheManager::init();
    StorageManager::loadSettings(g_settings);
    PomodoroStats::init();
    Logger::info("MAIN", "Settings loaded.");

    // 2. Initialize display
    if (DisplayManager::init()) {
        Serial.println("[MAIN] Display initialized.");
        DisplayManager::showStartupScreen();
    } else {
        Serial.println("[MAIN] Display init failed!");
    }

    // 3. Initialize buttons
    ButtonHandler::init();
    Serial.println("[MAIN] Buttons initialized.");

    // 4. Initialize shake detector
    ShakeDetector::init();
    Serial.println("[MAIN] Shake detector initialized.");

    // 5. Initialize Pomodoro with saved settings
    PomodoroManager::init(g_settings);
    PomodoroManager::updateSettings(g_pomodoro, g_settings);
    g_pomodoro.currentCycle = 0;
    g_pomodoro.phase = POMODORO_IDLE;
    g_pomodoro.savedPhase = POMODORO_IDLE;
    g_pomodoro.pendingBreakPhase = POMODORO_SHORT_BREAK;
    Serial.println("[MAIN] Pomodoro initialized.");

    // Audio is available even while the device is in Wi-Fi setup AP mode.
    if (AzanManager::init()) {
        AzanManager::setVolume(g_settings.volume);
    }

    // 6. Connect WiFi (or start AP for setup)
    DisplayManager::showConnectingScreen();
    g_wifiConnected = WifiManager::init(g_settings);

    if (g_wifiConnected) {
        Serial.println("[MAIN] WiFi connected.");

        // 7. Sync time via NTP
        g_timesynced = WifiManager::syncNTP(g_settings);

        // 8. Fetch prayer data
        g_prayerReady = PrayerManager::init(g_settings);
        if (g_prayerReady) {
            PrayerManager::getTodayPrayers(g_todayPrayers);
            PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, g_settings);
            Serial.println("[MAIN] Prayer data ready.");
        }

        // 9. Fetch weather
        g_weatherReady = WeatherManager::update(g_settings, g_weather);
        if (g_weatherReady) {
            Serial.println("[MAIN] Weather data ready.");
        }

        // 10. Initialize calendar
        CalendarManager::init(g_settings);
        CalendarManager::getHijriDate(g_hijriDate);
        CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
        Serial.println("[MAIN] Calendar initialized.");

        // 11. Initialize voice
        VoiceManager::init();
        Serial.println("[MAIN] Voice manager initialized.");

        // 12. Start web server
        WebServerManager::init(g_settings);
        Serial.println("[MAIN] Web server started.");
    } else {
        Serial.println("[MAIN] WiFi not connected. Starting AP mode...");
        if (!WifiManager::isAPMode()) {
            WifiManager::startAP();
        }
        WebServerManager::init(g_settings);
    }

    // 14. Draw initial screen
    ScreenRouter::draw(g_currentScreen);

    // 15. Create FreeRTOS tasks
    xTaskCreatePinnedToCore(taskDisplay,   "Display",   8192,  NULL, 2, &taskDisplayHandle,   1);
    xTaskCreatePinnedToCore(taskScheduler, "Scheduler", 8192,  NULL, 3, &taskSchedulerHandle, 0);
    xTaskCreatePinnedToCore(taskNetwork,   "Network",   16384, NULL, 1, &taskNetworkHandle,   0);
    xTaskCreatePinnedToCore(taskAudio,     "Audio",     8192,  NULL, 4, &taskAudioHandle,     1); // High priority audio on Core 1

    Serial.println("[MAIN] All tasks started. Prayer-Bot ready.");
    Serial.println("========================================");
}

// ============================================================
//  LOOP (unused - work is done in FreeRTOS tasks)
// ============================================================

void loop() {
    vTaskDelay(pdMS_TO_TICKS(1000));
}

// ============================================================
//  TASK: Display & UI
//  Handles screen updates, button input, and screen cycling.
// ============================================================

void taskDisplay(void* param) {
    TickType_t lastWake = xTaskGetTickCount();
    bool suppressButton2Actions = false;

    for (;;) {
        // Service OLED health
        DisplayManager::service();

        // Handle button presses
        ButtonHandler::update();
        int8_t webScreenRequest = g_webScreenRequest;
        g_webScreenRequest = -1;
        if (webScreenRequest >= 0 && webScreenRequest < SYS_SCREEN_COUNT) {
            g_currentScreen = (SystemScreen)webScreenRequest;
            g_screenStartMs = millis();
            ScreenRouter::draw(g_currentScreen);
        }

        uint8_t webPomodoroRequest = g_webPomodoroRequest;
        g_webPomodoroRequest = 0;
        if (webPomodoroRequest == 1 && g_azanState != AZAN_PLAYING) {
            PomodoroManager::togglePause(g_pomodoro);
        } else if (webPomodoroRequest == 2) {
            PomodoroManager::reset(g_pomodoro);
        }

        uint8_t webDhikrRequest = g_webDhikrRequest;
        g_webDhikrRequest = 0;
        if (webDhikrRequest > 0) {
            DhikrScreen::countTouch();
        }

        int16_t webVolumeRequest = g_webVolumeRequest;
        g_webVolumeRequest = -1;
        if (webVolumeRequest >= 0 && webVolumeRequest <= 100) {
            AzanManager::setVolume((uint8_t)webVolumeRequest);
        }

        bool anyTouch = ButtonHandler::isAnyTouchPressed();
        if (g_currentScreen == SYS_SCREEN_DHIKR && anyTouch) {
            DhikrScreen::countTouch();
        }

        bool startedPomodoroBreak = false;
        if (anyTouch &&
            g_pomodoro.phase == POMODORO_WAITING_FOR_BREAK &&
            g_azanState != AZAN_PLAYING) {
            PomodoroManager::startBreak(g_pomodoro);
            startedPomodoroBreak = true;
            suppressButton2Actions = ButtonHandler::isButton2Held();
        }

        if (anyTouch && g_azanState == AZAN_PLAYING) {
            g_azanStopRequested = true;
            suppressButton2Actions = ButtonHandler::isButton2Held();
        }

        if (ButtonHandler::isButton1Pressed() &&
            g_currentScreen != SYS_SCREEN_DHIKR &&
            !startedPomodoroBreak &&
            g_pomodoro.phase != POMODORO_WAITING_FOR_BREAK &&
            g_azanState != AZAN_PLAYING) {
            // Cycle to next screen
            g_currentScreen = (SystemScreen)((g_currentScreen + 1) % SYS_SCREEN_COUNT);
            g_screenStartMs = millis();
            ScreenRouter::draw(g_currentScreen);
        }

        if (ButtonHandler::isButton2LongPressed() &&
            !suppressButton2Actions &&
            g_currentScreen != SYS_SCREEN_DHIKR &&
            g_azanState != AZAN_PLAYING) {
            // Start voice mode
            if (g_voiceState == VOICE_IDLE && g_azanState == AZAN_IDLE) {
                g_voiceState = VOICE_LISTENING;
                VoiceManager::startListening();
            }
        }

        if (ButtonHandler::isButton2ShortPressed() &&
            !suppressButton2Actions &&
            g_currentScreen != SYS_SCREEN_DHIKR &&
            g_azanState != AZAN_PLAYING) {
            // Context action
            if (g_voiceState == VOICE_PLAYING) {
                VoiceManager::stopPlayback();
                g_voiceState = VOICE_IDLE;
            } else if (g_currentScreen == SYS_SCREEN_POMODORO) {
                PomodoroManager::togglePause(g_pomodoro);
            }
        }

        if (suppressButton2Actions &&
            !ButtonHandler::isButton2Held() &&
            !ButtonHandler::isButton2ShortPressed() &&
            !ButtonHandler::isButton2LongPressed()) {
            suppressButton2Actions = false;
        } else if (suppressButton2Actions &&
                   !ButtonHandler::isButton2Held() &&
                   (ButtonHandler::isButton2ShortPressed() ||
                    ButtonHandler::isButton2LongPressed())) {
            suppressButton2Actions = false;
        }

        // Update current screen content (clock tick, countdown, etc.)
        ScreenRouter::update(g_currentScreen);

        // Auto-slide logic for Dhikr screen (100 seconds)
        if (g_currentScreen == SYS_SCREEN_DHIKR &&
            g_azanState != AZAN_PLAYING &&
            (millis() - g_screenStartMs >= 100000)) {
            g_currentScreen = SYS_SCREEN_PRAYER_FOCUS; // Return to main screen
            g_screenStartMs = millis();
            ScreenRouter::draw(g_currentScreen);
        }

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(100));
    }
}

// ============================================================
//  TASK: Scheduler
//  Handles prayer time checks, azan triggering, shake
//  detection, Pomodoro ticks, and voice processing.
// ============================================================

void taskScheduler(void* param) {
    TickType_t lastWake = xTaskGetTickCount();
    PomodoroPhase previousPomodoroPhase = g_pomodoro.phase;

    for (;;) {
        // Update shake detector
        ShakeDetector::update();

        // Check if azan should play
        if (g_prayerReady && g_azanState == AZAN_IDLE && g_settings.azanEnabled) {
            if (PrayerManager::isAzanTime(g_todayPrayers)) {
                if (AzanManager::play(PrayerManager::getCurrentPrayerName())) {
                    g_azanState = AZAN_PLAYING;

                    // Pause Pomodoro during azan
                    if (g_pomodoro.phase == POMODORO_FOCUS ||
                        g_pomodoro.phase == POMODORO_SHORT_BREAK ||
                        g_pomodoro.phase == POMODORO_LONG_BREAK) {
                        PomodoroManager::pauseForAzan(g_pomodoro);
                    }
                }
            }
        }

        bool shakeDetected = ShakeDetector::isShakeDetected();
        bool touchStopRequested = g_azanStopRequested;
        g_azanStopRequested = false;
        if (g_azanState == AZAN_PLAYING && (shakeDetected || touchStopRequested)) {
            AzanManager::stop();
            g_azanState = AZAN_IDLE;
            resumePomodoroAfterAzan();
        }

        // Check if azan finished naturally
        if (g_azanState == AZAN_PLAYING && !AzanManager::isPlaying()) {
            g_azanState = AZAN_IDLE;
            resumePomodoroAfterAzan();
        }

        // Update prayer countdown
        if (g_prayerReady) {
            PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, g_settings);
        }

        // Update Pomodoro timer
        PomodoroManager::update(g_pomodoro);
        if (g_pomodoro.phase == POMODORO_FOCUS &&
            previousPomodoroPhase != POMODORO_FOCUS &&
            previousPomodoroPhase != POMODORO_PAUSED) {
            PomodoroStats::recordSessionStarted();
        }
        if (previousPomodoroPhase == POMODORO_FOCUS &&
            g_pomodoro.phase == POMODORO_WAITING_FOR_BREAK) {
            PomodoroStats::recordFocusCompleted(g_pomodoro.focusMinutes);
            AzanManager::playPomodoroBeep();
        }
        previousPomodoroPhase = g_pomodoro.phase;

        // Update voice state machine
        VoiceManager::update(g_voiceState);

        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(50));
    }
}

// ============================================================
//  TASK: Network
//  Handles periodic data refresh: weather, prayer cache,
//  NTP re-sync, and WiFi reconnection.
// ============================================================

void taskNetwork(void* param) {
    unsigned long lastWeatherUpdate = 0;  // Force immediate first fetch
    unsigned long lastPrayerUpdate  = 0;
    unsigned long lastNtpSync       = 0;
    unsigned long lastCacheCleanup  = 0;
    bool initialCacheCleanupPending = g_timesynced;
    bool networkServicesInitialized = g_wifiConnected;

    for (;;) {
        unsigned long now = millis();

        // Reconnect WiFi if needed
        if (WiFi.status() != WL_CONNECTED) {
            g_wifiConnected = WifiManager::reconnect();
        } else {
            g_wifiConnected = true;
        }

        if (g_wifiConnected && WiFi.status() == WL_CONNECTED) {
            if (!networkServicesInitialized) {
                Serial.println("[MAIN] WiFi connected after setup; initializing online services.");
                g_timesynced = WifiManager::syncNTP(g_settings);
                g_prayerReady = PrayerManager::init(g_settings);
                if (g_prayerReady) {
                    PrayerManager::getTodayPrayers(g_todayPrayers);
                    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, g_settings);
                }
                g_weatherReady = WeatherManager::update(g_settings, g_weather);
                CalendarManager::init(g_settings);
                CalendarManager::getHijriDate(g_hijriDate);
                CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                VoiceManager::init();
                networkServicesInitialized = true;
                initialCacheCleanupPending = g_timesynced;
                lastPrayerUpdate = millis();
                lastWeatherUpdate = lastPrayerUpdate;
                lastNtpSync = lastPrayerUpdate;
            }

            if (initialCacheCleanupPending) {
                CacheManager::cleanExpired(CACHE_MAX_AGE_DAYS);
                lastCacheCleanup = now;
                initialCacheCleanupPending = false;
            }

            if (g_ntpSyncRequested) {
                g_ntpSyncRequested = false;
                g_timesynced = WifiManager::syncNTP(g_settings);
                lastNtpSync = millis();
            }

            if (g_locationRefreshRequested) {
                g_locationRefreshRequested = false;
                g_prayerReady = PrayerManager::refresh(g_settings);
                if (g_prayerReady) {
                    PrayerManager::getTodayPrayers(g_todayPrayers);
                    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, g_settings);
                }
                lastPrayerUpdate = millis();
            }

            if (g_weatherRefreshRequested) {
                g_weatherRefreshRequested = false;
                g_weatherReady = WeatherManager::update(g_settings, g_weather);
                lastWeatherUpdate = millis();
            }

            // Refresh weather
            if ((now - lastWeatherUpdate) >= WEATHER_UPDATE_INTERVAL || lastWeatherUpdate == 0) {
                if (WeatherManager::update(g_settings, g_weather)) {
                    g_weatherReady = true;
                }
                lastWeatherUpdate = now;
            }

            // Refresh prayer data (daily / cache refresh)
            if ((now - lastPrayerUpdate) >= PRAYER_UPDATE_INTERVAL || lastPrayerUpdate == 0) {
                PrayerManager::getTodayPrayers(g_todayPrayers);
                CalendarManager::getHijriDate(g_hijriDate);
                CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                lastPrayerUpdate = now;
            }

            // NTP re-sync
            if ((now - lastNtpSync) >= NTP_SYNC_INTERVAL) {
                g_timesynced = WifiManager::syncNTP(g_settings);
                lastNtpSync = now;

                if (g_timesynced &&
                    (lastCacheCleanup == 0 ||
                     (now - lastCacheCleanup) >= CACHE_CLEANUP_INTERVAL)) {
                    CacheManager::cleanExpired(CACHE_MAX_AGE_DAYS);
                    lastCacheCleanup = now;
                }
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// ============================================================
//  TASK: Audio
//  High priority task dedicated entirely to feeding the MP3
//  decoder loop for stutter-free I2S audio playback.
// ============================================================

void taskAudio(void* param) {
    for (;;) {
        AzanManager::update();
        // Give time back to RTOS, small delay is enough to prevent WDT resets
        // while maintaining smooth playback.
        vTaskDelay(pdMS_TO_TICKS(2));
    }
}
