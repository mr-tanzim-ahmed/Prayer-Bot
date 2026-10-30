#include "logger.h"
#include <stdarg.h>

namespace Logger {

static uint8_t currentLevel = LOG_INFO;

void init(uint8_t level) {
    currentLevel = level;
    Serial.println("[LOGGER] Initialized.");
}

void setLevel(uint8_t level) {
    currentLevel = level;
}

static void printLog(const char* levelStr, const char* tag, const char* fmt, va_list args) {
    char buf[256];
    vsnprintf(buf, sizeof(buf), fmt, args);
    Serial.printf("[%s][%s] %s\n", levelStr, tag, buf);
}

void error(const char* tag, const char* fmt, ...) {
    if (currentLevel >= LOG_ERROR) {
        va_list args;
        va_start(args, fmt);
        printLog("E", tag, fmt, args);
        va_end(args);
    }
}

void warn(const char* tag, const char* fmt, ...) {
    if (currentLevel >= LOG_WARN) {
        va_list args;
        va_start(args, fmt);
        printLog("W", tag, fmt, args);
        va_end(args);
    }
}

void info(const char* tag, const char* fmt, ...) {
    if (currentLevel >= LOG_INFO) {
        va_list args;
        va_start(args, fmt);
        printLog("I", tag, fmt, args);
        va_end(args);
    }
}

void debug(const char* tag, const char* fmt, ...) {
    if (currentLevel >= LOG_DEBUG) {
        va_list args;
        va_start(args, fmt);
        printLog("D", tag, fmt, args);
        va_end(args);
    }
}

void logHeap(const char* tag) {
    if (currentLevel >= LOG_INFO) {
        Serial.printf("[HEAP][%s] free=%u largest=%u\n", 
                      tag, ESP.getFreeHeap(), ESP.getMaxAllocHeap());
    }
}

} // namespace Logger
