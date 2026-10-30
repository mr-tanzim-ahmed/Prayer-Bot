#include "screen_router.h"
#include "display_manager.h"
#include "screens/prayer_focus_screen.h"
#include "screens/all_prayers_screen.h"
#include "screens/clock_calendar_screen.h"
#include "screens/qibla_screen.h"
#include "compass_manager.h"
#include "screens/weather_screen.h"
#include "screens/pomodoro_screen.h"
#include "screens/dhikr_screen.h"
#include "screens/surah_screen.h"
#include "prayer_manager.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

// Access global state
extern Settings        g_settings;
extern SemaphoreHandle_t g_settingsMutex;
extern DailyPrayers    g_todayPrayers;
extern NextPrayerInfo  g_nextPrayer;
extern ProhibitedTimes g_prohibited;
extern HijriDate       g_hijriDate;
extern IslamicEvent    g_todayEvent;
extern volatile bool   g_prayerRefreshing;
extern volatile bool   g_locationRefreshRequested;
extern WeatherData     g_weather;
extern volatile bool   g_weatherRefreshRequested;
extern volatile bool   g_weatherRefreshing;
extern PomodoroState   g_pomodoro;
extern volatile AzanState g_azanState;
extern VoiceState      g_voiceState;
extern bool            g_weatherReady;
extern bool            g_prayerReady;

namespace {
    unsigned long lastQuranRedrawMs = 0;
    bool showingSahariIftar = false;
    bool allPrayersViewInitialized = false;
    bool azanOverlayDisplayed = false;
    float lastQiblaLatitude = DEFAULT_LATITUDE;
    float lastQiblaLongitude = DEFAULT_LONGITUDE;
    bool hasQiblaLocation = false;

    bool getQiblaLocation(float& latitude, float& longitude) {
        if (!g_settingsMutex) {
            latitude = g_settings.latitude;
            longitude = g_settings.longitude;
            return true;
        }
        if (xSemaphoreTake(g_settingsMutex, pdMS_TO_TICKS(50)) != pdTRUE)
            return false;
        latitude = g_settings.latitude;
        longitude = g_settings.longitude;
        xSemaphoreGive(g_settingsMutex);
        return true;
    }

    void drawQiblaScreen() {
        float latitude;
        float longitude;
        if (getQiblaLocation(latitude, longitude)) {
            lastQiblaLatitude = latitude;
            lastQiblaLongitude = longitude;
            hasQiblaLocation = true;
        } else if (!hasQiblaLocation) {
            Serial.println("[QIBLA] Could not read location settings.");
            return;
        }
        QiblaScreen::draw(lastQiblaLatitude, lastQiblaLongitude);
    }

    bool shouldShowSahariIftar(unsigned long timeOnScreen) {
        constexpr unsigned long CYCLE_MS = 5000UL;
        constexpr unsigned long PRAYER_TABLE_MS = 3000UL;
        return timeOnScreen % CYCLE_MS >= PRAYER_TABLE_MS;
    }
}

// Track time on current screen for animations
extern unsigned long   g_screenStartMs;

namespace ScreenRouter {

void draw(SystemScreen screen) {
    if (!DisplayManager::isAvailable()) return;

    if (g_azanState == AZAN_PLAYING) {
        azanOverlayDisplayed = true;
        PrayerFocusScreen::draw(g_nextPrayer, g_prohibited, true,
                                PrayerManager::getCurrentPrayerName(),
                                g_prayerRefreshing ||
                                    g_locationRefreshRequested);
        return;
    }
    
    unsigned long timeOnScreen = millis() - g_screenStartMs;

    switch (screen) {
        case SYS_SCREEN_PRAYER_FOCUS:
            PrayerFocusScreen::draw(g_nextPrayer, g_prohibited,
                                    g_azanState == AZAN_PLAYING,
                                    PrayerManager::getCurrentPrayerName(),
                                    g_prayerRefreshing ||
                                        g_locationRefreshRequested);
            break;

        case SYS_SCREEN_ALL_PRAYERS:
            showingSahariIftar = shouldShowSahariIftar(timeOnScreen);
            allPrayersViewInitialized = true;
            if (showingSahariIftar) {
                AllPrayersScreen::drawSahariIftar(g_todayPrayers);
            } else {
                AllPrayersScreen::draw(g_todayPrayers, g_nextPrayer);
            }
            break;

        case SYS_SCREEN_CLOCK_CALENDAR:
            ClockCalendarScreen::draw(g_hijriDate, g_todayEvent, g_todayPrayers);
            break;

        case SYS_SCREEN_QIBLA:
            drawQiblaScreen();
            break;

        case SYS_SCREEN_WEATHER:
            WeatherScreen::draw(g_weather,
                                g_weatherRefreshing || g_weatherRefreshRequested,
                                timeOnScreen);
            break;

        case SYS_SCREEN_POMODORO:
            PomodoroScreen::draw(g_pomodoro);
            break;

        case SYS_SCREEN_DHIKR:
            DhikrScreen::draw(timeOnScreen);
            break;

        case SYS_SCREEN_QURAN:
            SurahScreen::draw();
            break;

        default:
            break;
    }
}

void transitionOut() {
    if (!DisplayManager::isAvailable() || g_azanState == AZAN_PLAYING) return;

    constexpr uint8_t TRANSITION_STEPS = 2;
    U8G2& oled = DisplayManager::getDisplay();
    for (uint8_t step = 1; step <= TRANSITION_STEPS; ++step) {
        oled.setDrawColor(0);
        oled.drawBox(0, 0,
                     (SCREEN_WIDTH * step) / TRANSITION_STEPS,
                     SCREEN_HEIGHT);
        oled.setDrawColor(1);
        DisplayManager::sendBuffer();
    }
}

void update(SystemScreen screen) {
    if (!DisplayManager::isAvailable()) return;

    if (g_azanState == AZAN_PLAYING) {
        azanOverlayDisplayed = true;
        PrayerFocusScreen::update(g_nextPrayer, g_prohibited, true,
                                  PrayerManager::getCurrentPrayerName(),
                                  g_prayerRefreshing ||
                                      g_locationRefreshRequested);
        return;
    }

    if (azanOverlayDisplayed) {
        azanOverlayDisplayed = false;
        draw(screen);
        return;
    }

    unsigned long timeOnScreen = millis() - g_screenStartMs;

    switch (screen) {
        case SYS_SCREEN_PRAYER_FOCUS:
            PrayerFocusScreen::update(g_nextPrayer, g_prohibited,
                                      g_azanState == AZAN_PLAYING,
                                      PrayerManager::getCurrentPrayerName(),
                                      g_prayerRefreshing ||
                                          g_locationRefreshRequested);
            break;

        case SYS_SCREEN_ALL_PRAYERS: {
            const bool shouldShowSahari = shouldShowSahariIftar(timeOnScreen);
            if (!allPrayersViewInitialized ||
                shouldShowSahari != showingSahariIftar) {
                showingSahariIftar = shouldShowSahari;
                allPrayersViewInitialized = true;
                if (showingSahariIftar) {
                    AllPrayersScreen::drawSahariIftar(g_todayPrayers);
                } else {
                    AllPrayersScreen::draw(g_todayPrayers, g_nextPrayer);
                }
            }
            break;
        }

        case SYS_SCREEN_CLOCK_CALENDAR:
            ClockCalendarScreen::update(g_hijriDate, g_todayEvent, g_todayPrayers);
            break;

        case SYS_SCREEN_QIBLA:
            drawQiblaScreen();
            break;

        case SYS_SCREEN_WEATHER:
            WeatherScreen::update(
                g_weather, g_weatherRefreshing || g_weatherRefreshRequested,
                timeOnScreen);
            break;

        case SYS_SCREEN_POMODORO:
            PomodoroScreen::update(g_pomodoro);
            break;

        case SYS_SCREEN_DHIKR:
            DhikrScreen::update(timeOnScreen);
            break;

        case SYS_SCREEN_QURAN:
            if (millis() - lastQuranRedrawMs >= 100) {
                lastQuranRedrawMs = millis();
                SurahScreen::draw();
            }
            break;

        default:
            break;
    }
}

} // namespace ScreenRouter
