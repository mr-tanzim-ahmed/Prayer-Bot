#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "core/types.h"

// ============================================================
//  WEB SERVER MANAGER
//  Serves the phone dashboard and REST API for settings.
//  Runs on the ESP32's async web server.
// ============================================================

namespace WebServerManager {
    // Start the web server
    void init(Settings& settings);

    // Stop the web server
    void stop();

    // Check if server is running
    bool isRunning();

    // Print the dashboard URLs for the active Wi-Fi interfaces
    void logDashboardAddresses();

    // Apply one queued dashboard control from the display task.
    void processDashboardControl();
}

#endif // WEB_SERVER_MANAGER_H
