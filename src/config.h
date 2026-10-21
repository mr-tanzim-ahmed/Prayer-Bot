#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <driver/i2s.h>
#include "secrets.h"

// ============================================================
// PRAYER-BOT CONFIGURATION
// Central configuration for hardware, timing, APIs and defaults.
// ============================================================

// ------------------------------------------------------------
// Hardware Pin Assignments
// ------------------------------------------------------------

// OLED Display (I2C)
#define PIN_OLED_SDA          11
#define PIN_OLED_SCL          12
#define OLED_I2C_ADDRESS      0x3C
#define OLED_USE_SH1106       1

// Capacitive touch sensor
#define PIN_TOUCH_SENSOR      4

// MPU6050 IMU (shared I2C bus with OLED)
#define MPU6050_I2C_ADDRESS   0x68

// I2S Microphone (INMP441)
#define PIN_MIC_SCK           16
#define PIN_MIC_WS            17
#define PIN_MIC_SD            18

// I2S Amplifier (MAX98357A)
#define PIN_AMP_BCLK          5
#define PIN_AMP_LRC           6
#define PIN_AMP_DIN           7

// ------------------------------------------------------------
// I2C Configuration
// ------------------------------------------------------------
#define I2C_CLOCK_SPEED       400000UL
#define OLED_CHECK_INTERVAL   2000UL

// ------------------------------------------------------------
// Display
// ------------------------------------------------------------
#define SCREEN_WIDTH          128
#define SCREEN_HEIGHT         64

// ------------------------------------------------------------
// Timing Intervals
// ------------------------------------------------------------
#define WEATHER_UPDATE_INTERVAL   (30UL * 60UL * 1000UL)
#define PRAYER_FETCH_INTERVAL     (30UL * 60UL * 1000UL)
#define PRAYER_UPDATE_INTERVAL    (60UL * 1000UL)
#define NTP_SYNC_INTERVAL         (6UL * 3600UL * 1000UL)
#define TOUCH_DEBOUNCE_MS         50UL

// ------------------------------------------------------------
// Shake Detection (MPU6050)
// ------------------------------------------------------------
#define SHAKE_THRESHOLD       15000
#define SHAKE_COUNT_REQUIRED  3
#define SHAKE_WINDOW_MS       1000UL

// ------------------------------------------------------------
// Audio
// ------------------------------------------------------------
#define I2S_MIC_PORT          I2S_NUM_0
#define I2S_AMP_PORT          I2S_NUM_1
#define MIC_SAMPLE_RATE       16000
#define MIC_RECORD_SECONDS    3
#define AMP_SAMPLE_RATE       44100
#define DEFAULT_VOLUME        80

// ------------------------------------------------------------
// WiFi & Network
// ------------------------------------------------------------
#define WIFI_CONNECT_TIMEOUT  35000UL
#define AP_SSID               "Prayer-Bot"
#define AP_PASSWORD           "12345678"


#define WEB_SERVER_PORT       80
#define WEB_SERVER_HTTPS_PORT  443
#define NTP_SERVER            "pool.ntp.org"

// ------------------------------------------------------------
// API Endpoints
// ------------------------------------------------------------
#define ALADHAN_API_BASE      "https://api.aladhan.com/v1"
#define MP3QURAN_API_BASE     "https://mp3quran.net/api/v3"
#define OPENWEATHER_API_BASE  "https://api.openweathermap.org/data/2.5"

// API keys are declared in secrets.h.
// Existing code can continue using:
// DEFAULT_GEMINI_API_KEY
// DEFAULT_OWM_API_KEY

// ------------------------------------------------------------
// Default Location (Dhaka, Bangladesh)
// ------------------------------------------------------------
#define DEFAULT_CITY_NAME     "Dhaka"
#define DEFAULT_LATITUDE      23.8103f
#define DEFAULT_LONGITUDE     90.4125f
#define DEFAULT_TIMEZONE      "Asia/Dhaka"
#define DEFAULT_UTC_OFFSET    6.0f

// ------------------------------------------------------------
// Prayer Calculation Defaults
// ------------------------------------------------------------
#define DEFAULT_CALC_METHOD   3
#define DEFAULT_ASR_SCHOOL    1
#define DEFAULT_HIJRI_OFFSET  0

// Default Dhaka Base Prayer Times (fallback before network fetch)
#define DEFAULT_DHAKA_FAJR_H     4
#define DEFAULT_DHAKA_FAJR_M     41
#define DEFAULT_DHAKA_SUNRISE_H  5
#define DEFAULT_DHAKA_SUNRISE_M  57
#define DEFAULT_DHAKA_DHUHR_H    11
#define DEFAULT_DHAKA_DHUHR_M    44
#define DEFAULT_DHAKA_ASR_H      15
#define DEFAULT_DHAKA_ASR_M      53
#define DEFAULT_DHAKA_MAGHRIB_H  17
#define DEFAULT_DHAKA_MAGHRIB_M  30
#define DEFAULT_DHAKA_ISHA_H     18
#define DEFAULT_DHAKA_ISHA_M     41

// ------------------------------------------------------------
// Pomodoro Defaults
// ------------------------------------------------------------
#define DEFAULT_FOCUS_MIN     25
#define DEFAULT_SHORT_BREAK   5
#define DEFAULT_LONG_BREAK    15
#define DEFAULT_CYCLES        4

// ------------------------------------------------------------
// Prohibited Time Offsets (minutes)
// ------------------------------------------------------------
#define DEFAULT_SUNRISE_OFFSET   15
#define DEFAULT_ZAWAL_OFFSET     6
#define DEFAULT_SUNSET_OFFSET    15

// ------------------------------------------------------------
// LittleFS Paths
// ------------------------------------------------------------
#define SETTINGS_PATH         "/settings.json"
#define PRAYER_CACHE_PATH     "/prayer_cache.json"
#define WEATHER_CACHE_PATH    "/weather_cache.json"
#define FESTIVAL_TABLE_PATH   "/festivals.json"

// ------------------------------------------------------------
// Cache Management
// ------------------------------------------------------------
#define CACHE_META_PATH        "/cache_meta.json"
#define CACHE_MAX_AGE_DAYS     7
#define CACHE_PRAYER_MAX_DAYS  35
#define CACHE_WEATHER_MAX_DAYS 1
#define CACHE_FESTIVAL_MAX_DAYS 365
#define CACHE_CLEANUP_INTERVAL (24UL * 3600UL * 1000UL)

// ------------------------------------------------------------
// Quran
// ------------------------------------------------------------
#define TOTAL_SURAHS          114

#endif // CONFIG_H