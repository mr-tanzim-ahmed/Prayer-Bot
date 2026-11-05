#ifndef PRAYER_FOCUS_SCREEN_H
#define PRAYER_FOCUS_SCREEN_H

#include "core/types.h"

// ============================================================
//  PRAYER FOCUS SCREEN (Screen 1 - Default)
//  Shows the current prayer briefly, then the next prayer and
//  its start/end times with a countdown.
// ============================================================

namespace PrayerFocusScreen {
    void draw(const NextPrayerInfo& next, const ProhibitedTimes& prohibited,
              bool azanPlaying, PrayerName activePrayer, bool syncing,
              bool showNextPrayer);
    void update(const NextPrayerInfo& next, const ProhibitedTimes& prohibited,
                bool azanPlaying, PrayerName activePrayer, bool syncing,
                bool showNextPrayer);
}

#endif // PRAYER_FOCUS_SCREEN_H
