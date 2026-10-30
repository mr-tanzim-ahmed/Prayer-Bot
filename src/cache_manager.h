#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include <Arduino.h>

namespace CacheManager {
    void init();
    bool isCacheValid(const char* path, uint32_t maxAgeDays);
    void markUpdated(const char* path);
    void cleanExpired(uint32_t maxAgeDays);
    void cleanAll();
    uint32_t getCacheAge(const char* path);
    size_t getTotalCacheSize();
    void listCacheFiles();
}

#endif // CACHE_MANAGER_H
