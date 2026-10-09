#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include "types.h"

// ============================================================
//  STORAGE MANAGER
//  Manages LittleFS for persistent settings, prayer cache,
//  weather cache, and festival data.
// ============================================================

namespace StorageManager {
    // Initialize LittleFS
    bool init();

    // Settings
    bool loadSettings(Settings& settings);
    bool saveSettings(const Settings& settings);
    void loadDefaults(Settings& settings);

    // Generic file read/write
    String readFile(const char* path);
    bool writeFile(const char* path, const String& content);
    bool exists(const char* path);
}

#endif // STORAGE_MANAGER_H
