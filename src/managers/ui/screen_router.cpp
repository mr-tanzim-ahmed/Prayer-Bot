#include "managers/ui/screen_router.h"
#include "managers/device/display_manager.h"
#include "screens/prayer_focus_screen.h"
#include "screens/all_prayers_screen.h"
#include "screens/clock_calendar_screen.h"
#include "screens/qibla_screen.h"
#include "managers/device/compass_manager.h"
#include "screens/weather_screen.h"
#include "screens/pomodoro_screen.h"
#include "screens/dhikr_screen.h"
#include "screens/surah_screen.h"
#include "managers/prayer/prayer_manager.h"
#include "core/shared_state.h"
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
    constexpr unsigned long PRAYER_CURRENT_PAGE_MS = 4000UL;
    constexpr unsigned long PRAYER_NEXT_PAGE_MS = 4000UL;
    constexpr unsigned long PRAYER_PAGE_CYCLE_MS =
        PRAYER_CURRENT_PAGE_MS + PRAYER_NEXT_PAGE_MS;
    bool pomodoroViewInitialized = false;
    PomodoroPhase lastPomodoroPhase = POMODORO_IDLE;
    int32_t lastPomodoroRemainingSeconds = -1;
    uint8_t lastPomodoroCycle = 0;
    bool lastPomodoroAzanPaused = false;
    uint16_t lastPomodoroFocusMinutes = 0;
    uint16_t lastPomodoroShortBreakMinutes = 0;
    uint16_t lastPomodoroLongBreakMinutes = 0;
    uint8_t lastPomodoroAnimationFrame = 0;
    float lastQiblaLatitude = DEFAULT_LATITUDE;
    float lastQiblaLongitude = DEFAULT_LONGITUDE;
    bool hasQiblaLocation = true;

    bool copyPrayerSnapshot(DailyPrayers& prayers, NextPrayerInfo& nextPrayer,
                            ProhibitedTimes& prohibited) {
        if (!lockPrayerData()) return false;
        prayers = g_todayPrayers;
        nextPrayer = g_nextPrayer;
        prohibited = g_prohibited;
        unlockPrayerData();
        return true;
    }

    bool copyCalendarSnapshot(HijriDate& hijriDate, IslamicEvent& todayEvent) {
        if (!lockCalendarData()) return false;
        hijriDate = g_hijriDate;
        todayEvent = g_todayEvent;
        unlockCalendarData();
        return true;
    }

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

    bool shouldShowNextPrayer(unsigned long timeOnScreen) {
        return timeOnScreen % PRAYER_PAGE_CYCLE_MS >=
            PRAYER_CURRENT_PAGE_MS;
    }

    void drawPomodoro() {
        PomodoroState state;
        if (!lockPomodoroState()) return;
        state = g_pomodoro;
        unlockPomodoroState();
        PomodoroScreen::draw(state);
        lastPomodoroPhase = state.phase;
        lastPomodoroRemainingSeconds = state.remainingSeconds;
        lastPomodoroCycle = state.currentCycle;
        lastPomodoroAzanPaused = state.pausedByAzan;
        lastPomodoroFocusMinutes = state.focusMinutes;
        lastPomodoroShortBreakMinutes = state.shortBreakMinutes;
        lastPomodoroLongBreakMinutes = state.longBreakMinutes;
        lastPomodoroAnimationFrame =
            static_cast<uint8_t>((millis() / 350UL) % 8UL);
        pomodoroViewInitialized = true;
    }
}

// Track time on current screen for animations
extern unsigned long   g_screenStartMs;

namespace ScreenRouter {

void draw(SystemScreen screen) {
    if (!DisplayManager::isAvailable()) return;

    if (g_azanState == AZAN_PLAYING) {
        DailyPrayers prayers;
        NextPrayerInfo nextPrayer;
        ProhibitedTimes prohibited;
        if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) return;
        azanOverlayDisplayed = true;
        PrayerFocusScreen::draw(prayers, nextPrayer, prohibited, true,
                                PrayerManager::getCurrentPrayerName(),
                                g_prayerRefreshing ||
                                    g_locationRefreshRequested,
                                false);
        return;
    }
    
    unsigned long timeOnScreen = millis() - g_screenStartMs;
    const bool showNextPrayer = shouldShowNextPrayer(timeOnScreen);

    switch (screen) {
        case SYS_SCREEN_PRAYER_FOCUS: {
            DailyPrayers prayers;
            NextPrayerInfo nextPrayer;
            ProhibitedTimes prohibited;
            if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) break;
            PrayerFocusScreen::draw(prayers, nextPrayer, prohibited,
                                    g_azanState == AZAN_PLAYING,
                                    PrayerManager::getCurrentPrayerName(),
                                    g_prayerRefreshing ||
                                        g_locationRefreshRequested,
                                    showNextPrayer);
            break;
        }

        case SYS_SCREEN_ALL_PRAYERS: {
            DailyPrayers prayers;
            NextPrayerInfo nextPrayer;
            ProhibitedTimes prohibited;
            if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) break;
            (void)prohibited;
            showingSahariIftar = shouldShowSahariIftar(timeOnScreen);
            allPrayersViewInitialized = true;
            if (showingSahariIftar) {
                AllPrayersScreen::drawSahariIftar(prayers);
            } else {
                AllPrayersScreen::draw(prayers, nextPrayer);
            }
            break;
        }

        case SYS_SCREEN_CLOCK_CALENDAR: {
            DailyPrayers prayers;
            NextPrayerInfo nextPrayer;
            ProhibitedTimes prohibited;
            HijriDate hijriDate;
            IslamicEvent todayEvent;
            if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) break;
            if (!copyCalendarSnapshot(hijriDate, todayEvent)) break;
            (void)nextPrayer;
            (void)prohibited;
            ClockCalendarScreen::draw(hijriDate, todayEvent, prayers);
            break;
        }

        case SYS_SCREEN_QIBLA:
            drawQiblaScreen();
            break;

        case SYS_SCREEN_WEATHER:
            WeatherScreen::draw(g_weather,
                                g_weatherRefreshing || g_weatherRefreshRequested,
                                timeOnScreen);
            break;

        case SYS_SCREEN_POMODORO:
            drawPomodoro();
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

void update(SystemScreen screen) {
    if (!DisplayManager::isAvailable()) return;

    if (g_azanState == AZAN_PLAYING) {
        DailyPrayers prayers;
        NextPrayerInfo nextPrayer;
        ProhibitedTimes prohibited;
        if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) return;
        azanOverlayDisplayed = true;
        PrayerFocusScreen::update(prayers, nextPrayer, prohibited, true,
                                  PrayerManager::getCurrentPrayerName(),
                                  g_prayerRefreshing ||
                                      g_locationRefreshRequested,
                                  false);
        return;
    }

    if (azanOverlayDisplayed) {
        azanOverlayDisplayed = false;
        draw(screen);
        return;
    }

    unsigned long timeOnScreen = millis() - g_screenStartMs;
    const bool showNextPrayer = shouldShowNextPrayer(timeOnScreen);

    switch (screen) {
        case SYS_SCREEN_PRAYER_FOCUS: {
            DailyPrayers prayers;
            NextPrayerInfo nextPrayer;
            ProhibitedTimes prohibited;
            if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) break;
            PrayerFocusScreen::update(prayers, nextPrayer, prohibited,
                                      g_azanState == AZAN_PLAYING,
                                      PrayerManager::getCurrentPrayerName(),
                                      g_prayerRefreshing ||
                                          g_locationRefreshRequested,
                                      showNextPrayer);
            break;
        }

        case SYS_SCREEN_ALL_PRAYERS: {
            DailyPrayers prayers;
            NextPrayerInfo nextPrayer;
            ProhibitedTimes prohibited;
            if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) break;
            (void)prohibited;
            const bool shouldShowSahari = shouldShowSahariIftar(timeOnScreen);
            if (!allPrayersViewInitialized ||
                shouldShowSahari != showingSahariIftar) {
                showingSahariIftar = shouldShowSahari;
                allPrayersViewInitialized = true;
                if (showingSahariIftar) {
                    AllPrayersScreen::drawSahariIftar(prayers);
                } else {
                    AllPrayersScreen::draw(prayers, nextPrayer);
                }
            }
            break;
        }

        case SYS_SCREEN_CLOCK_CALENDAR: {
            DailyPrayers prayers;
            NextPrayerInfo nextPrayer;
            ProhibitedTimes prohibited;
            HijriDate hijriDate;
            IslamicEvent todayEvent;
            if (!copyPrayerSnapshot(prayers, nextPrayer, prohibited)) break;
            if (!copyCalendarSnapshot(hijriDate, todayEvent)) break;
            (void)nextPrayer;
            (void)prohibited;
            ClockCalendarScreen::update(hijriDate, todayEvent, prayers);
            break;
        }

        case SYS_SCREEN_QIBLA:
            drawQiblaScreen();
            break;

        case SYS_SCREEN_WEATHER:
            WeatherScreen::update(
                g_weather, g_weatherRefreshing || g_weatherRefreshRequested,
                timeOnScreen);
            break;

        case SYS_SCREEN_POMODORO: {
            PomodoroState state;
            if (!lockPomodoroState()) break;
            state = g_pomodoro;
            unlockPomodoroState();
            const uint8_t animationFrame =
                static_cast<uint8_t>((millis() / 350UL) % 8UL);
            const bool animatedPhase =
                state.phase == POMODORO_IDLE ||
                state.phase == POMODORO_WAITING_FOR_BREAK;
            if (!pomodoroViewInitialized ||
                state.phase != lastPomodoroPhase ||
                state.remainingSeconds != lastPomodoroRemainingSeconds ||
                state.currentCycle != lastPomodoroCycle ||
                state.pausedByAzan != lastPomodoroAzanPaused ||
                state.focusMinutes != lastPomodoroFocusMinutes ||
                state.shortBreakMinutes != lastPomodoroShortBreakMinutes ||
                state.longBreakMinutes != lastPomodoroLongBreakMinutes ||
                (animatedPhase &&
                 animationFrame != lastPomodoroAnimationFrame)) {
                PomodoroScreen::draw(state);
                lastPomodoroPhase = state.phase;
                lastPomodoroRemainingSeconds = state.remainingSeconds;
                lastPomodoroCycle = state.currentCycle;
                lastPomodoroAzanPaused = state.pausedByAzan;
                lastPomodoroFocusMinutes = state.focusMinutes;
                lastPomodoroShortBreakMinutes = state.shortBreakMinutes;
                lastPomodoroLongBreakMinutes = state.longBreakMinutes;
                lastPomodoroAnimationFrame = animationFrame;
                pomodoroViewInitialized = true;
            }
            break;
        }

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
