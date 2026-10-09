#ifndef WEB_SERVER_MANAGER_H
#define WEB_SERVER_MANAGER_H

#include "types.h"

// ============================================================
//  WEB SERVER MANAGER
//  Serves the phone dashboard and REST API for settings.
//  Runs on the ESP32-S3's async web server.
// ============================================================

namespace WebServerManager {
    // Start the web server
    void init(Settings& settings);

    // Stop the web server
    void stop();

    // Check if server is running
    bool isRunning();
}

#endif // WEB_SERVER_MANAGER_H
