#ifndef CALENDAR_MANAGER_H
#define CALENDAR_MANAGER_H

#include "types.h"

// ============================================================
//  CALENDAR MANAGER
//  Manages Hijri date, Islamic month names, and the
//  festival/important day table.
// ============================================================

namespace CalendarManager {
    // Initialize with settings (loads festival table)
    void init(const Settings& settings);

    // Get current Hijri date (from cached API data + offset)
    void getHijriDate(HijriDate& date);

    // Check if today matches any Islamic event
    void getTodayEvent(const HijriDate& date, IslamicEvent& event);

    // Check if an event is coming within N days
    bool isEventComingSoon(const HijriDate& date, IslamicEvent& event, uint8_t withinDays = 3);

    // Update Hijri offset (from dashboard)
    void setHijriOffset(int8_t offset);
}

#endif // CALENDAR_MANAGER_H
