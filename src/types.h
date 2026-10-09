#ifndef TYPES_H
#define TYPES_H

#include <Arduino.h>

// ============================================================
//  PRAYER-BOT DATA TYPES
//  Shared data structures used across modules.
// ============================================================

// ------------------------------------------------------------
//  Prayer Data
// ------------------------------------------------------------

struct PrayerTime {
    uint8_t hour;
    uint8_t minute;
};

struct DailyPrayers {
    PrayerTime fajr;
    PrayerTime sunrise;
    PrayerTime dhuhr;
    PrayerTime asr;
    PrayerTime maghrib;
    PrayerTime isha;
};

enum PrayerName : uint8_t {
    PRAYER_FAJR = 0,
    PRAYER_SUNRISE,
    PRAYER_DHUHR,
    PRAYER_ASR,
    PRAYER_MAGHRIB,
    PRAYER_ISHA,
    PRAYER_NONE
};

struct NextPrayerInfo {
    PrayerName  name;
    PrayerTime  startTime;
    int32_t     countdownSeconds;    // Seconds until this prayer starts
    int32_t     remainingSeconds;    // Seconds left to pray current prayer (window)
    bool        windowOpen;          // Is a prayer window currently open?
    PrayerName  currentPrayer;       // Which prayer window is currently open
};

struct ProhibitedWindow {
    uint8_t startHour;
    uint8_t startMinute;
    uint8_t endHour;
    uint8_t endMinute;
    bool    active;                  // Is this window currently active?
};

struct ProhibitedTimes {
    ProhibitedWindow afterSunrise;
    ProhibitedWindow atZawal;
    ProhibitedWindow beforeSunset;
    ProhibitedWindow nextWindow;     // The soonest upcoming or current
};

// ------------------------------------------------------------
//  Hijri Calendar
// ------------------------------------------------------------

struct HijriDate {
    uint16_t year;
    uint8_t  month;
    uint8_t  day;
    String   monthName;              // e.g. "Ramadan"
    String   designation;            // e.g. "AH"
};

struct IslamicEvent {
    uint8_t  month;
    uint8_t  day;
    String   name;
    bool     isWeekly;               // true for Jumuah
};

// ------------------------------------------------------------
//  Weather
// ------------------------------------------------------------

struct WeatherData {
    float   temperatureC;
    int     humidity;
    int     weatherId;               // OpenWeatherMap weather condition ID
    String  weatherMain;             // "Clear", "Clouds", "Rain", etc.
    String  weatherDescription;      // "light rain", etc.
    String  iconCode;                // OpenWeather icon code
    int     airQualityIndex;         // 1-5 (OpenWeather) or US AQI
    float   pm25;
    String  cityName;
    unsigned long lastUpdateMs;      // millis() of last successful update
    bool    valid;
};

// Weather icon types for OLED drawing
enum WeatherIconType : uint8_t {
    ICON_SUN = 0,
    ICON_CLOUD,
    ICON_RAIN,
    ICON_STORM,
    ICON_SNOW,
    ICON_FOG
};

// ------------------------------------------------------------
//  Pomodoro
// ------------------------------------------------------------

enum PomodoroPhase : uint8_t {
    POMODORO_IDLE = 0,
    POMODORO_FOCUS,
    POMODORO_SHORT_BREAK,
    POMODORO_LONG_BREAK,
    POMODORO_PAUSED
};

struct PomodoroState {
    PomodoroPhase phase;
    uint16_t focusMinutes;
    uint16_t shortBreakMinutes;
    uint16_t longBreakMinutes;
    uint8_t  totalCycles;
    uint8_t  currentCycle;
    unsigned long phaseStartMs;
    unsigned long pausedElapsedMs;
    int32_t  remainingSeconds;
    bool     pausedByAzan;           // Auto-paused during azan
};

// ------------------------------------------------------------
//  Settings (persisted to LittleFS)
// ------------------------------------------------------------

struct Settings {
    // Location
    char    cityName[64];
    float   latitude;
    float   longitude;
    char    timezone[40];
    float   utcOffset;

    // Prayer calculation
    uint8_t calcMethod;
    uint8_t asrSchool;
    int8_t  hijriOffset;             // -2 to +2

    // Azan
    bool    azanEnabled;
    uint8_t volume;                  // 0-100

    // Pomodoro
    uint16_t focusMinutes;
    uint16_t shortBreakMinutes;
    uint16_t longBreakMinutes;
    uint8_t  pomodoroCycles;

    // Prohibited time offsets (minutes)
    uint8_t sunriseOffset;
    uint8_t zawalOffset;
    uint8_t sunsetOffset;

    // OpenWeatherMap API key (user-provided)
    char    owmApiKey[48];
};

// ------------------------------------------------------------
//  System State
// ------------------------------------------------------------

enum SystemScreen : uint8_t {
    SYS_SCREEN_PRAYER_FOCUS = 0,
    SYS_SCREEN_ALL_PRAYERS,
    SYS_SCREEN_CLOCK_CALENDAR,
    SYS_SCREEN_WEATHER,
    SYS_SCREEN_POMODORO,
    SYS_SCREEN_COUNT
};

enum AzanState : uint8_t {
    AZAN_IDLE = 0,
    AZAN_PLAYING,
    AZAN_STOPPING
};

enum VoiceState : uint8_t {
    VOICE_IDLE = 0,
    VOICE_LISTENING,
    VOICE_PROCESSING,
    VOICE_PLAYING,
    VOICE_ERROR
};

#endif // TYPES_H
