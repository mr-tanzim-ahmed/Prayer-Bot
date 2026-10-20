#ifndef SCREEN_ROUTER_H
#define SCREEN_ROUTER_H

#include "types.h"

// ============================================================
//  SCREEN ROUTER
//  Routes display drawing to the correct screen module
//  based on the current SystemScreen.
// ============================================================

namespace ScreenRouter {
    // Draw the specified screen (full redraw)
    void draw(SystemScreen screen);

    // Briefly wipe the previous screen before drawing a new selection.
    void transitionOut();

    // Update the current screen (partial update, e.g. clock tick)
    void update(SystemScreen screen);
}

#endif // SCREEN_ROUTER_H
