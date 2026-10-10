#include "screen_router.h"
#include "display_manager.h"
#include "screens/prayer_focus_screen.h"
#include "screens/all_prayers_screen.h"
#include "screens/clock_calendar_screen.h"
#include "screens/weather_screen.h"
#include "screens/pomodoro_screen.h"
#include "screens/dhikr_screen.h"
#include "prayer_manager.h"

// Access global state
extern DailyPrayers    g_todayPrayers;
extern NextPrayerInfo  g_nextPrayer;
extern ProhibitedTimes g_prohibited;
extern HijriDate       g_hijriDate;
extern IslamicEvent    g_todayEvent;
extern WeatherData     g_weather;
extern PomodoroState   g_pomodoro;
extern volatile AzanState g_azanState;
extern VoiceState      g_voiceState;
extern bool            g_weatherReady;
extern bool            g_prayerReady;

// Track time on current screen for animations
extern unsigned long   g_screenStartMs;

namespace ScreenRouter {

void draw(SystemScreen screen) {
    if (!DisplayManager::isAvailable()) return;

    if (g_azanState == AZAN_PLAYING) {
        PrayerFocusScreen::draw(g_nextPrayer, g_prohibited, true,
                                PrayerManager::getCurrentPrayerName());
        return;
    }
    
    unsigned long timeOnScreen = millis() - g_screenStartMs;

    switch (screen) {
        case SYS_SCREEN_PRAYER_FOCUS:
            PrayerFocusScreen::draw(g_nextPrayer, g_prohibited,
                                    g_azanState == AZAN_PLAYING,
                                    PrayerManager::getCurrentPrayerName());
            break;

        case SYS_SCREEN_ALL_PRAYERS:
            AllPrayersScreen::draw(g_todayPrayers, g_nextPrayer);
            break;

        case SYS_SCREEN_CLOCK_CALENDAR:
            ClockCalendarScreen::draw(g_hijriDate, g_todayEvent, g_todayPrayers);
            break;

        case SYS_SCREEN_WEATHER:
            WeatherScreen::draw(g_weather);
            break;

        case SYS_SCREEN_POMODORO:
            PomodoroScreen::draw(g_pomodoro);
            break;

        case SYS_SCREEN_DHIKR:
            DhikrScreen::draw(timeOnScreen);
            break;

        default:
            break;
    }
}

void update(SystemScreen screen) {
    if (!DisplayManager::isAvailable()) return;

    if (g_azanState == AZAN_PLAYING) {
        PrayerFocusScreen::update(g_nextPrayer, g_prohibited, true,
                                  PrayerManager::getCurrentPrayerName());
        return;
    }

    unsigned long timeOnScreen = millis() - g_screenStartMs;

    switch (screen) {
        case SYS_SCREEN_PRAYER_FOCUS:
            PrayerFocusScreen::update(g_nextPrayer, g_prohibited,
                                      g_azanState == AZAN_PLAYING,
                                      PrayerManager::getCurrentPrayerName());
            break;

        case SYS_SCREEN_CLOCK_CALENDAR:
            ClockCalendarScreen::update(g_hijriDate, g_todayEvent, g_todayPrayers);
            break;

        case SYS_SCREEN_WEATHER:
            // Weather screen is mostly static, redrawn on data change
            break;

        case SYS_SCREEN_POMODORO:
            PomodoroScreen::update(g_pomodoro);
            break;

        case SYS_SCREEN_DHIKR:
            DhikrScreen::update(timeOnScreen);
            break;

        default:
            break;
    }
}

} // namespace ScreenRouter
