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
    String getSetupAPSSID();
    bool saveCredentials(const String& ssid, const String& password);

    // Begin a station connection without interrupting the setup access point.
    bool connectToNetwork(const String& ssid, const String& password);
    void restartAfterConnection();
    bool shouldRestartAfterConnection();
    bool didConnectionAttemptFail();

    // Reconnect after disconnection
    bool reconnect();

    // Read-only connection state; isConnecting() also finalizes saved credentials.
    bool connectionInProgress();
    void abortConnection();

    // Sync time via NTP
    bool syncNTP(const Settings& settings);

    // Status
    bool isConnected();
    bool isConnecting();
    String getIP();
    bool isAPMode();
}

#endif // WIFI_MANAGER_H
