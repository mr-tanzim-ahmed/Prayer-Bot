#ifndef DHIKR_SCREEN_H
#define DHIKR_SCREEN_H

#include "types.h"

// ============================================================
//  DHIKR SCREEN (Screen 6)
//  Automatically loops through 5 Dhikr phrases, each showing
//  for 20 seconds.
// ============================================================

namespace DhikrScreen {
    // Draw the dhikr screen based on elapsed time on this screen
    void draw(unsigned long timeOnScreenMs);
    void update(unsigned long timeOnScreenMs);
    void countTouch();
    uint32_t getCount();
}

#endif // DHIKR_SCREEN_H
