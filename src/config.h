#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <driver/i2s.h>

// ============================================================
//  PRAYER-BOT CONFIGURATION
//  Central configuration for all hardware pins, timing
//  constants, API endpoints, and default values.
// ============================================================

// ------------------------------------------------------------
//  Hardware Pin Assignments
// ------------------------------------------------------------

// OLED Display (I2C)
#define PIN_OLED_SDA          8
#define PIN_OLED_SCL          9
#define OLED_I2C_ADDRESS      0x3C
#define OLED_USE_SH1106       1        // 1 = SH1106, 0 = SSD1306

// Touch Buttons
#define PIN_BUTTON_1          4        // Screen cycle
#define PIN_BUTTON_2          5        // Voice / context action

// MPU6050 IMU (I2C, shared bus with OLED)
#define MPU6050_I2C_ADDRESS   0x68

// I2S Microphone (INMP441)
#define PIN_MIC_SCK           16
#define PIN_MIC_WS            17
#define PIN_MIC_SD            18

// I2S Amplifier (MAX98357A)
#define PIN_AMP_BCLK          12
#define PIN_AMP_LRC           13
#define PIN_AMP_DIN           14

// ------------------------------------------------------------
//  I2C Configuration
// ------------------------------------------------------------
#define I2C_CLOCK_SPEED       400000UL
#define OLED_CHECK_INTERVAL   2000UL   // ms between OLED health checks

// ------------------------------------------------------------
//  Display
// ------------------------------------------------------------
#define SCREEN_WIDTH          128
#define SCREEN_HEIGHT         64
#define TOTAL_SCREENS         6        // Prayer Focus, All Prayers, Clock/Calendar, Weather, Pomodoro, Dhikr

// Screen indices
#define SCREEN_PRAYER_FOCUS   0
#define SCREEN_ALL_PRAYERS    1
#define SCREEN_CLOCK_CALENDAR 2
#define SCREEN_WEATHER        3
#define SCREEN_POMODORO       4
#define SCREEN_DHIKR          5

// ------------------------------------------------------------
//  Timing Intervals
// ------------------------------------------------------------
#define WEATHER_UPDATE_INTERVAL   (30UL * 60UL * 1000UL)   // 30 minutes
#define PRAYER_UPDATE_INTERVAL    (60UL * 1000UL)           // 1 minute - recheck next prayer
#define PRAYER_CACHE_FETCH_DAYS   30                        // Cache a month of prayer data
#define NTP_SYNC_INTERVAL         (6UL * 3600UL * 1000UL)  // 6 hours
#define CLOCK_DISPLAY_INTERVAL    1000UL                    // 1 second
#define BUTTON_DEBOUNCE_MS        200UL
#define BUTTON_LONG_PRESS_MS      5000UL                    // 5 seconds for voice mode

// ------------------------------------------------------------
//  Shake Detection (MPU6050)
// ------------------------------------------------------------
#define SHAKE_THRESHOLD       15000    // Acceleration magnitude threshold
#define SHAKE_COUNT_REQUIRED  3        // Number of peaks within the window
#define SHAKE_WINDOW_MS       1000UL   // Time window for shake detection

// ------------------------------------------------------------
//  Audio
// ------------------------------------------------------------
#define I2S_MIC_PORT          I2S_NUM_0
#define I2S_AMP_PORT          I2S_NUM_1
#define MIC_SAMPLE_RATE       16000
#define MIC_RECORD_SECONDS    5
#define AMP_SAMPLE_RATE       44100
#define DEFAULT_VOLUME        80       // 0-100

// ------------------------------------------------------------
//  WiFi & Network
// ------------------------------------------------------------
#define WIFI_CONNECT_TIMEOUT  20000UL  // 20 seconds
#define AP_SSID               "PrayerBot-Setup"
#define AP_PASSWORD           "12345678"
#define WEB_SERVER_PORT       80
#define NTP_SERVER            "pool.ntp.org"

// ------------------------------------------------------------
//  API Endpoints
// ------------------------------------------------------------
#define ALADHAN_API_BASE      "https://api.aladhan.com/v1"
#define OPEN_METEO_WEATHER    "https://api.open-meteo.com/v1/forecast"
#define OPEN_METEO_AIR        "https://air-quality-api.open-meteo.com/v1/air-quality"
#define QURAN_AUDIO_BASE      "https://cdn.islamic.network/quran/audio/128/ar.alafasy"

// OpenWeatherMap (for weather - to be replaced with Open-Meteo)
#define OPENWEATHER_API_BASE  "https://api.openweathermap.org/data/2.5"

// ------------------------------------------------------------
//  Default Location (Dhaka, Bangladesh)
// ------------------------------------------------------------
#define DEFAULT_CITY_NAME     "Dhaka"
#define DEFAULT_LATITUDE      23.8103f
#define DEFAULT_LONGITUDE     90.4125f
#define DEFAULT_TIMEZONE      "Asia/Dhaka"
#define DEFAULT_UTC_OFFSET    6.0f     // UTC+6

// ------------------------------------------------------------
//  Prayer Calculation Defaults
// ------------------------------------------------------------
#define DEFAULT_CALC_METHOD   3        // Muslim World League
#define DEFAULT_ASR_SCHOOL    1        // 1 = Hanafi
#define DEFAULT_HIJRI_OFFSET  0        // -2 to +2

// ------------------------------------------------------------
//  Pomodoro Defaults
// ------------------------------------------------------------
#define DEFAULT_FOCUS_MIN     25
#define DEFAULT_SHORT_BREAK   5
#define DEFAULT_LONG_BREAK    15
#define DEFAULT_CYCLES        4

// ------------------------------------------------------------
//  Prohibited Time Offsets (minutes)
// ------------------------------------------------------------
#define DEFAULT_SUNRISE_OFFSET   15    // minutes after sunrise
#define DEFAULT_ZAWAL_OFFSET     6     // minutes around solar noon
#define DEFAULT_SUNSET_OFFSET    15    // minutes before sunset

// ------------------------------------------------------------
//  LittleFS Paths
// ------------------------------------------------------------
#define SETTINGS_PATH         "/settings.json"
#define PRAYER_CACHE_PATH     "/prayer_cache.json"
#define WEATHER_CACHE_PATH    "/weather_cache.json"
#define FESTIVAL_TABLE_PATH   "/festivals.json"

// ------------------------------------------------------------
//  Surah Count
// ------------------------------------------------------------
#define TOTAL_SURAHS          114

#endif // CONFIG_H
