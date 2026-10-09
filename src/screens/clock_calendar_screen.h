#ifndef CLOCK_CALENDAR_SCREEN_H
#define CLOCK_CALENDAR_SCREEN_H

#include "types.h"

// ============================================================
//  CLOCK & CALENDAR SCREEN (Screen 3)
//  Shows current time, Gregorian date, Hijri date and month,
//  and a banner for any Islamic festival or important day.
// ============================================================

namespace ClockCalendarScreen {
    void draw(const HijriDate& hijri, const IslamicEvent& event);
    void update(const HijriDate& hijri, const IslamicEvent& event);
}

#endif // CLOCK_CALENDAR_SCREEN_H
