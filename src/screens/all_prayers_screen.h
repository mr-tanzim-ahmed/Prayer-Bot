#ifndef ALL_PRAYERS_SCREEN_H
#define ALL_PRAYERS_SCREEN_H

#include "types.h"

// ============================================================
//  ALL PRAYERS SCREEN (Screen 2)
//  Shows Fajr, Dhuhr, Asr, Maghrib, Isha times + Sunrise,
//  with the next prayer highlighted.
// ============================================================

namespace AllPrayersScreen {
    void draw(const DailyPrayers& prayers, const NextPrayerInfo& next);
}

#endif // ALL_PRAYERS_SCREEN_H
