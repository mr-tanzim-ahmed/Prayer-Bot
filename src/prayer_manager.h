#ifndef PRAYER_MANAGER_H
#define PRAYER_MANAGER_H

#include "types.h"

// ============================================================
//  PRAYER MANAGER
//  Fetches, caches, and computes prayer times. Provides
//  next-prayer info, countdown, prayer windows, and
//  prohibited time calculations.
// ============================================================

namespace PrayerManager {
    // Initialize: load cache or fetch from API
    bool init(const Settings& settings);

    // Get today's prayer times
    void getTodayPrayers(DailyPrayers& prayers);

    // Calculate next prayer, countdown, window, prohibited times
    void updateNextPrayer(NextPrayerInfo& next, ProhibitedTimes& prohibited,
                          const Settings& settings);

    // Check if it's time to play azan (within ~30 seconds of prayer time)
    bool isAzanTime(const DailyPrayers& prayers);

    // Get the name of the current prayer (for azan type selection)
    PrayerName getCurrentPrayerName();

    // Force re-fetch from API (e.g. after city change)
    bool refresh(const Settings& settings);

    // Get prayer name as string
    const char* getPrayerNameStr(PrayerName name);
}

#endif // PRAYER_MANAGER_H
