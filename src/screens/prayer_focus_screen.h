#ifndef PRAYER_FOCUS_SCREEN_H
#define PRAYER_FOCUS_SCREEN_H

#include "types.h"

// ============================================================
//  PRAYER FOCUS SCREEN (Screen 1 - Default)
//  Shows only the next prayer: name, start time, countdown,
//  time remaining in current window, and next prohibited time.
// ============================================================

namespace PrayerFocusScreen {
    void draw(const NextPrayerInfo& next, const ProhibitedTimes& prohibited);
    void update(const NextPrayerInfo& next, const ProhibitedTimes& prohibited);
}

#endif // PRAYER_FOCUS_SCREEN_H
