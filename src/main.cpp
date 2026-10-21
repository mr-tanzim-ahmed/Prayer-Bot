#include <Arduino.h>
#include <WiFi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
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
SemaphoreHandle_t g_settingsMutex = nullptr;
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
volatile bool   g_webAzanStartRequested = false;
volatile bool   g_locationRefreshRequested = false;
volatile bool   g_prayerRefreshing = false;
volatile bool   g_weatherRefreshRequested = false;
volatile bool   g_weatherRefreshing = false;
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
TaskHandle_t taskVoiceHandle     = NULL;

// --- Task functions ---
void taskDisplay(void* param);
void taskScheduler(void* param);
void taskNetwork(void* param);
void taskAudio(void* param);
void taskVoice(void* param);

namespace {
    void changeScreen(SystemScreen screen) {
        if (screen == g_currentScreen) return;
        ScreenRouter::transitionOut();
        if (g_currentScreen == SYS_SCREEN_QURAN)
            VoiceManager::leaveSurahScreen();
        AzanManager::playScreenChangeBeep();
        g_currentScreen = screen;
        g_screenStartMs = millis();
        if (screen == SYS_SCREEN_QURAN)
            VoiceManager::enterSurahScreen();
        if (screen == SYS_SCREEN_WEATHER && g_wifiConnected)
            g_weatherRefreshRequested = true;
        if (screen == SYS_SCREEN_PRAYER_FOCUS && g_wifiConnected)
            g_locationRefreshRequested = true;
        ScreenRouter::draw(g_currentScreen);
    }

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
    Serial.println("   Mumins Personal Assistant");
    Serial.println("========================================");

    // 1. Initialize storage and load settings
    Logger::init(LOG_DEBUG);
    const bool storageReady = StorageManager::init();
    CacheManager::init();
    const bool settingsLoaded = StorageManager::loadSettings(g_settings);
    PomodoroStats::init();
    g_settingsMutex = xSemaphoreCreateMutex();
    if (!g_settingsMutex) {
        Logger::error("MAIN", "Could not create settings lock; dashboard settings will be read-only.");
    }
    if (!storageReady) {
        Logger::error("STORAGE", "Persistent storage unavailable; settings cannot be saved.");
    } else if (!settingsLoaded) {
        Logger::warn("STORAGE", "Settings unavailable or invalid; safe defaults are active.");
    } else {
        Logger::info("MAIN", "Settings loaded.");
    }

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
    VoiceManager::init();

    // 6. Initialize prayer & calendar base data unconditionally (cached / Dhaka defaults)
    g_prayerReady = PrayerManager::init(g_settings);
    PrayerManager::getTodayPrayers(g_todayPrayers);
    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, g_settings);
    CalendarManager::init(g_settings);
    CalendarManager::getHijriDate(g_hijriDate);
    CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
    Serial.println("[MAIN] Prayer data and calendar initialized.");

    // 7. Connect WiFi (or start AP for setup)
    DisplayManager::showConnectingScreen();
    g_wifiConnected = WifiManager::init(g_settings);

    if (g_wifiConnected) {
        Serial.println("[MAIN] WiFi connected.");

        // Sync time via NTP
        g_timesynced = WifiManager::syncNTP(g_settings);

        // Fetch live prayer data
        if (PrayerManager::refresh(g_settings)) {
            PrayerManager::getTodayPrayers(g_todayPrayers);
            PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, g_settings);
            CalendarManager::getHijriDate(g_hijriDate);
            CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
            Serial.println("[MAIN] Live prayer data fetched.");
        }

        // Fetch weather
        g_weatherReady = WeatherManager::update(g_settings, g_weather);
        if (g_weatherReady) {
            Serial.println("[MAIN] Weather data ready.");
        }

        // Start web server
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
    if (xTaskCreatePinnedToCore(taskDisplay, "Display", 8192, NULL, 2,
                                &taskDisplayHandle, 1) != pdPASS) {
        Logger::error("MAIN", "Could not start the display task.");
    }
    if (xTaskCreatePinnedToCore(taskScheduler, "Scheduler", 8192, NULL, 3,
                                &taskSchedulerHandle, 0) != pdPASS) {
        Logger::error("MAIN", "Could not start the scheduler task.");
    }
    if (xTaskCreatePinnedToCore(taskNetwork, "Network", 16384, NULL, 1,
                                &taskNetworkHandle, 0) != pdPASS) {
        Logger::error("MAIN", "Could not start the network task.");
    }
    if (xTaskCreatePinnedToCore(taskAudio, "Audio", 8192, NULL, 4,
                                &taskAudioHandle, 1) != pdPASS) {
        Logger::error("MAIN", "Could not start the audio task.");
    }
    if (xTaskCreatePinnedToCore(taskVoice, "Voice", 16384, NULL, 1,
                                &taskVoiceHandle, 0) != pdPASS) {
        Logger::error("MAIN", "Could not start the voice task.");
    }

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

    for (;;) {
        // Service OLED health
        DisplayManager::service();

        // Handle button presses
        ButtonHandler::update();
        WifiManager::processCaptivePortal();
        int8_t webScreenRequest = g_webScreenRequest;
        g_webScreenRequest = -1;
        if (webScreenRequest >= 0 && webScreenRequest < SYS_SCREEN_COUNT) {
            changeScreen((SystemScreen)webScreenRequest);
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

        bool touchPressed = ButtonHandler::isTouchPressed();
        bool touchLongPressed = ButtonHandler::isTouchLongPressed();
        if (g_currentScreen == SYS_SCREEN_DHIKR && touchPressed) {
            DhikrScreen::countTouch();
        }

        if (g_currentScreen == SYS_SCREEN_POMODORO &&
            touchLongPressed &&
            g_azanState != AZAN_PLAYING) {
            if (g_pomodoro.phase == POMODORO_IDLE) {
                PomodoroManager::start(g_pomodoro);
            } else if (g_pomodoro.phase == POMODORO_WAITING_FOR_BREAK) {
                PomodoroManager::reset(g_pomodoro);
            } else if (!g_pomodoro.pausedByAzan) {
                PomodoroManager::togglePause(g_pomodoro);
            } else {
                Logger::info("POMODORO", "Timer remains paused until the azan pause ends.");
            }
        }

        if (g_currentScreen == SYS_SCREEN_QURAN &&
            touchLongPressed &&
            g_azanState != AZAN_PLAYING) {
            if (VoiceManager::isPlaying()) {
                VoiceManager::stopPlayback();
            } else {
                VoiceManager::playSurah(VoiceManager::getPlayingSurah());
            }
        }

        if (touchPressed &&
            g_pomodoro.phase == POMODORO_WAITING_FOR_BREAK &&
            g_azanState != AZAN_PLAYING) {
            PomodoroManager::startBreak(g_pomodoro);
        }

        if (touchPressed &&
            g_currentScreen == SYS_SCREEN_POMODORO &&
            g_pomodoro.phase == POMODORO_PAUSED &&
            !g_pomodoro.pausedByAzan &&
            g_azanState != AZAN_PLAYING) {
            PomodoroManager::togglePause(g_pomodoro);
        }

        if (touchPressed && g_azanState == AZAN_PLAYING) {
            g_azanStopRequested = true;
        }

        if (touchPressed && g_azanState != AZAN_PLAYING) {
            changeScreen((SystemScreen)((g_currentScreen + 1) % SYS_SCREEN_COUNT));
        }

        // Update current screen content (clock tick, countdown, etc.)
        ScreenRouter::update(g_currentScreen);

        // Auto-slide logic for Dhikr screen (100 seconds)
        if (g_currentScreen == SYS_SCREEN_DHIKR &&
            g_azanState != AZAN_PLAYING &&
            (millis() - g_screenStartMs >= 100000)) {
            changeScreen(SYS_SCREEN_PRAYER_FOCUS);
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
    Settings taskSettings = g_settings;

    for (;;) {
        if (g_settingsMutex &&
            xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            taskSettings = g_settings;
            xSemaphoreGive(g_settingsMutex);
        }

        // Update shake detector
        ShakeDetector::update();

        const bool manualAzanRequest = g_webAzanStartRequested;
        g_webAzanStartRequested = false;
        const bool scheduledAzanRequest = g_prayerReady &&
            taskSettings.azanEnabled &&
            PrayerManager::isAzanTime(g_todayPrayers);
        if (g_azanState == AZAN_IDLE &&
            (scheduledAzanRequest || manualAzanRequest)) {
            const PrayerName prayer = g_prayerReady
                ? PrayerManager::getCurrentPrayerName()
                : PRAYER_FAJR;
            if (AzanManager::play(prayer)) {
                g_azanState = AZAN_PLAYING;

                if (g_pomodoro.phase == POMODORO_FOCUS ||
                    g_pomodoro.phase == POMODORO_SHORT_BREAK ||
                    g_pomodoro.phase == POMODORO_LONG_BREAK) {
                    PomodoroManager::pauseForAzan(g_pomodoro);
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
        } else if (shakeDetected && g_azanState != AZAN_PLAYING) {
            if (g_currentScreen == SYS_SCREEN_QURAN) {
                if (VoiceManager::isPlaying()) {
                    VoiceManager::stopPlayback();
                }
            } else if (g_currentScreen == SYS_SCREEN_POMODORO) {
                switch (g_pomodoro.phase) {
                    case POMODORO_IDLE:
                        PomodoroManager::start(g_pomodoro);
                        break;
                    case POMODORO_FOCUS:
                    case POMODORO_SHORT_BREAK:
                    case POMODORO_LONG_BREAK:
                        PomodoroManager::togglePause(g_pomodoro);
                        break;
                    case POMODORO_PAUSED:
                        if (!g_pomodoro.pausedByAzan) {
                            PomodoroManager::togglePause(g_pomodoro);
                        }
                        break;
                    case POMODORO_WAITING_FOR_BREAK:
                    default:
                        break;
                }
            }
        }

        // Check if azan finished naturally
        if (g_azanState == AZAN_PLAYING && !AzanManager::isPlaying()) {
            g_azanState = AZAN_IDLE;
            resumePomodoroAfterAzan();
        }

        // Update prayer countdown & prohibited windows continuously
        PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, taskSettings);

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
        } else if ((previousPomodoroPhase == POMODORO_SHORT_BREAK ||
                    previousPomodoroPhase == POMODORO_LONG_BREAK) &&
                   g_pomodoro.phase == POMODORO_FOCUS) {
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
    unsigned long lastPrayerFetch = 0;
    unsigned long lastPrayerUpdate  = 0;
    unsigned long lastNtpSync       = 0;
    unsigned long lastCacheCleanup  = 0;
    bool initialCacheCleanupPending = g_timesynced;
    bool networkServicesInitialized = g_wifiConnected;
    Settings taskSettings = g_settings;

    for (;;) {
        unsigned long now = millis();
        if (g_settingsMutex &&
            xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            taskSettings = g_settings;
            xSemaphoreGive(g_settingsMutex);
        }

        WifiManager::isConnecting();

        // Reconnect WiFi if needed
        if (WiFi.status() != WL_CONNECTED) {
            g_wifiConnected = WifiManager::reconnect();
        } else {
            g_wifiConnected = true;
        }

        if (WifiManager::shouldRestartAfterConnection()) {
            Serial.println("[MAIN] Router connection verified; restarting to apply saved Wi-Fi credentials.");
            vTaskDelay(pdMS_TO_TICKS(1200));
            ESP.restart();
        }

        if (g_wifiConnected && WiFi.status() == WL_CONNECTED) {
            if (!networkServicesInitialized) {
                Serial.println("[MAIN] WiFi connected; initializing online services.");
                g_timesynced = WifiManager::syncNTP(taskSettings);
                g_prayerRefreshing = true;
                g_prayerReady = PrayerManager::refresh(taskSettings);
                g_prayerRefreshing = false;
                if (g_prayerReady) {
                    PrayerManager::getTodayPrayers(g_todayPrayers);
                    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, taskSettings);
                    CalendarManager::getHijriDate(g_hijriDate);
                    CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                }
                g_weatherRefreshing = true;
                g_weatherReady = WeatherManager::update(taskSettings, g_weather);
                g_weatherRefreshing = false;
                networkServicesInitialized = true;
                initialCacheCleanupPending = g_timesynced;
                lastPrayerUpdate = millis();
                lastPrayerFetch = lastPrayerUpdate;
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
                g_timesynced = WifiManager::syncNTP(taskSettings);
                lastNtpSync = millis();
            }

            if (g_locationRefreshRequested) {
                g_locationRefreshRequested = false;
                g_prayerRefreshing = true;
                g_prayerReady = PrayerManager::refresh(taskSettings);
                g_prayerRefreshing = false;
                if (g_prayerReady) {
                    PrayerManager::getTodayPrayers(g_todayPrayers);
                    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, taskSettings);
                    CalendarManager::getHijriDate(g_hijriDate);
                    CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                }
                lastPrayerUpdate = millis();
                lastPrayerFetch = lastPrayerUpdate;
            }

            if (g_weatherRefreshRequested) {
                g_weatherRefreshRequested = false;
                g_weatherRefreshing = true;
                g_weatherReady = WeatherManager::update(taskSettings, g_weather);
                g_weatherRefreshing = false;
                lastWeatherUpdate = millis();
            }

            if (!PrayerManager::isLiveSynced() &&
                (now - lastPrayerUpdate) >= 8000UL) {
                g_prayerRefreshing = true;
                if (PrayerManager::refresh(taskSettings)) {
                    g_prayerReady = true;
                    PrayerManager::getTodayPrayers(g_todayPrayers);
                    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, taskSettings);
                    CalendarManager::getHijriDate(g_hijriDate);
                    CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                    lastPrayerFetch = millis();
                    Serial.println("[MAIN] Live prayer data fetched from Aladhan API.");
                }
                g_prayerRefreshing = false;
                lastPrayerUpdate = now;
            }

            if ((now - lastPrayerFetch) >= PRAYER_FETCH_INTERVAL) {
                g_prayerRefreshing = true;
                g_prayerReady = PrayerManager::refresh(taskSettings);
                g_prayerRefreshing = false;
                if (g_prayerReady) {
                    PrayerManager::getTodayPrayers(g_todayPrayers);
                    PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited,
                                                    taskSettings);
                    CalendarManager::getHijriDate(g_hijriDate);
                    CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                }
                lastPrayerFetch = millis();
                lastPrayerUpdate = lastPrayerFetch;
            }

            // Refresh weather
            if ((now - lastWeatherUpdate) >= WEATHER_UPDATE_INTERVAL || lastWeatherUpdate == 0) {
                g_weatherRefreshing = true;
                if (WeatherManager::update(taskSettings, g_weather)) {
                    g_weatherReady = true;
                }
                g_weatherRefreshing = false;
                lastWeatherUpdate = now;
            }

            // Refresh prayer & prohibited window status every minute
            if ((now - lastPrayerUpdate) >= PRAYER_UPDATE_INTERVAL || lastPrayerUpdate == 0) {
                PrayerManager::getTodayPrayers(g_todayPrayers);
                PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited, taskSettings);
                CalendarManager::getHijriDate(g_hijriDate);
                CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                lastPrayerUpdate = now;
            }

            // NTP re-sync
            const unsigned long ntpRetryInterval = g_timesynced
                ? NTP_SYNC_INTERVAL
                : 5UL * 60UL * 1000UL;
            if ((now - lastNtpSync) >= ntpRetryInterval) {
                g_timesynced = WifiManager::syncNTP(taskSettings);
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

void taskVoice(void* param) {
    Settings taskSettings = g_settings;
    for (;;) {
        if (g_settingsMutex &&
            xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(50)) == pdTRUE) {
            taskSettings = g_settings;
            xSemaphoreGive(g_settingsMutex);
        }
        VoiceManager::processPending(taskSettings);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
