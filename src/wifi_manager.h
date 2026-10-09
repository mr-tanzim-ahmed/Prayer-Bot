#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include "types.h"

// ============================================================
//  WIFI MANAGER
//  Handles WiFi connection, AP mode for setup, NTP sync,
//  and reconnection logic.
// ============================================================

namespace WifiManager {
    // Connect to WiFi using settings, or start AP if no creds
    bool init(const Settings& settings);

    // Start AP mode for WiFi provisioning
    void startAP();

    // Reconnect after disconnection
    bool reconnect();

    // Sync time via NTP
    bool syncNTP(const Settings& settings);

    // Status
    bool isConnected();
    String getIP();
    bool isAPMode();
}

#endif // WIFI_MANAGER_H
