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
AzanState       g_azanState     = AZAN_IDLE;
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
    StorageManager::init();
    StorageManager::loadSettings(g_settings);
    Serial.println("[MAIN] Settings loaded.");

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
    Serial.println("[MAIN] Pomodoro initialized.");

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
            PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited);
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

        // 11. Initialize azan
        AzanManager::init();
        Serial.println("[MAIN] Azan manager initialized.");

        // 12. Initialize voice
        VoiceManager::init();
        Serial.println("[MAIN] Voice manager initialized.");

        // 13. Start web server
        WebServerManager::init(g_settings);
        Serial.println("[MAIN] Web server started.");
    } else {
        Serial.println("[MAIN] WiFi not connected. Starting AP mode...");
        WifiManager::startAP();
        WebServerManager::init(g_settings);
    }

    // 14. Draw initial screen
    ScreenRouter::draw(g_currentScreen);

    // 15. Create FreeRTOS tasks
    xTaskCreatePinnedToCore(taskDisplay,   "Display",   8192,  NULL, 2, &taskDisplayHandle,   1);
    xTaskCreatePinnedToCore(taskScheduler, "Scheduler", 8192,  NULL, 3, &taskSchedulerHandle, 0);
    xTaskCreatePinnedToCore(taskNetwork,   "Network",   16384, NULL, 1, &taskNetworkHandle,   0);

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

        if (ButtonHandler::isButton1Pressed()) {
            // Cycle to next screen
            g_currentScreen = (SystemScreen)((g_currentScreen + 1) % SYS_SCREEN_COUNT);
            ScreenRouter::draw(g_currentScreen);
        }

        if (ButtonHandler::isButton2LongPressed()) {
            // Start voice mode
            if (g_voiceState == VOICE_IDLE && g_azanState == AZAN_IDLE) {
                g_voiceState = VOICE_LISTENING;
                VoiceManager::startListening();
            }
        }

        if (ButtonHandler::isButton2ShortPressed()) {
            // Context action
            if (g_voiceState == VOICE_PLAYING) {
                VoiceManager::stopPlayback();
                g_voiceState = VOICE_IDLE;
            } else if (g_currentScreen == SYS_SCREEN_POMODORO) {
                PomodoroManager::togglePause(g_pomodoro);
            }
        }

        // Update current screen content (clock tick, countdown, etc.)
        ScreenRouter::update(g_currentScreen);

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

    for (;;) {
        // Update shake detector
        ShakeDetector::update();

        // Check if azan should play
        if (g_prayerReady && g_azanState == AZAN_IDLE && g_settings.azanEnabled) {
            if (PrayerManager::isAzanTime(g_todayPrayers)) {
                g_azanState = AZAN_PLAYING;
                AzanManager::play(PrayerManager::getCurrentPrayerName());

                // Pause Pomodoro during azan
                if (g_pomodoro.phase == POMODORO_FOCUS ||
                    g_pomodoro.phase == POMODORO_SHORT_BREAK ||
                    g_pomodoro.phase == POMODORO_LONG_BREAK) {
                    PomodoroManager::pauseForAzan(g_pomodoro);
                }
            }
        }

        // Check shake to stop azan
        if (g_azanState == AZAN_PLAYING && ShakeDetector::isShakeDetected()) {
            AzanManager::stop();
            g_azanState = AZAN_IDLE;

            // Resume Pomodoro if it was paused by azan
            if (g_pomodoro.pausedByAzan) {
                PomodoroManager::resumeFromAzan(g_pomodoro);
            }
        }

        // Check if azan finished naturally
        if (g_azanState == AZAN_PLAYING && !AzanManager::isPlaying()) {
            g_azanState = AZAN_IDLE;
            if (g_pomodoro.pausedByAzan) {
                PomodoroManager::resumeFromAzan(g_pomodoro);
            }
        }

        // Update prayer countdown
        if (g_prayerReady) {
            PrayerManager::updateNextPrayer(g_nextPrayer, g_prohibited);
        }

        // Update Pomodoro timer
        PomodoroManager::update(g_pomodoro);

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
    unsigned long lastWeatherUpdate = millis();
    unsigned long lastPrayerUpdate  = millis();
    unsigned long lastNtpSync       = millis();

    for (;;) {
        unsigned long now = millis();

        // Reconnect WiFi if needed
        if (WiFi.status() != WL_CONNECTED) {
            g_wifiConnected = WifiManager::reconnect();
        }

        if (g_wifiConnected) {
            // Refresh weather
            if ((now - lastWeatherUpdate) >= WEATHER_UPDATE_INTERVAL) {
                if (WeatherManager::update(g_settings, g_weather)) {
                    g_weatherReady = true;
                }
                lastWeatherUpdate = now;
            }

            // Refresh prayer data (daily / cache refresh)
            if ((now - lastPrayerUpdate) >= PRAYER_UPDATE_INTERVAL) {
                PrayerManager::getTodayPrayers(g_todayPrayers);
                CalendarManager::getHijriDate(g_hijriDate);
                CalendarManager::getTodayEvent(g_hijriDate, g_todayEvent);
                lastPrayerUpdate = now;
            }

            // NTP re-sync
            if ((now - lastNtpSync) >= NTP_SYNC_INTERVAL) {
                WifiManager::syncNTP(g_settings);
                lastNtpSync = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
