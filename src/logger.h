#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

#define LOG_NONE  0
#define LOG_ERROR 1
#define LOG_WARN  2
#define LOG_INFO  3
#define LOG_DEBUG 4

namespace Logger {
    void init(uint8_t level = LOG_INFO);
    void setLevel(uint8_t level);
    void error(const char* tag, const char* fmt, ...);
    void warn(const char* tag, const char* fmt, ...);
    void info(const char* tag, const char* fmt, ...);
    void debug(const char* tag, const char* fmt, ...);
    void logHeap(const char* tag);
}

#endif // LOGGER_H
