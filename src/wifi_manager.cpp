#include "wifi_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <WiFi.h>

// ============================================================
//  WIFI MANAGER IMPLEMENTATION
// ============================================================

namespace {
    bool apMode = false;
    bool connectionPending = false;
    unsigned long connectionStartedMs = 0;
    String storedSSID;
    String storedPassword;
}

namespace WifiManager {

bool init(const Settings& settings) {
    String ssid = StorageManager::readFile("/wifi_ssid.txt");
    String pass = StorageManager::readFile("/wifi_pass.txt");

    if (ssid.isEmpty()) {
        Serial.println("[WiFi] No saved credentials. Starting AP...");
        startAP();
        return false;
    }

    storedSSID = ssid;
    storedPassword = pass;

    WiFi.mode(WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(ssid.c_str(), pass.c_str());

    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           (millis() - start) < WIFI_CONNECT_TIMEOUT) {
        delay(250);
        Serial.print(".");
    }
    Serial.println();

    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[WiFi] Connection failed.");
        return false;
    }

    Serial.print("[WiFi] Connected. IP: ");
    Serial.println(WiFi.localIP());
    apMode = false;
    return true;
}

void startAP() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    apMode = true;

    Serial.print("[WiFi] AP started: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
}

bool connectToNetwork(const String& ssid, const String& password) {
    if (ssid.isEmpty()) return false;
    storedSSID = ssid;
    storedPassword = password;

    WiFi.mode(apMode ? WIFI_AP_STA : WIFI_STA);
    WiFi.setSleep(false);
    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
    connectionStartedMs = millis();
    connectionPending = true;
    Serial.printf("[WiFi] Connecting to configured network: %s\n", storedSSID.c_str());
    return true;
}

bool reconnect() {
    if (apMode) return false;
    if (WiFi.status() == WL_CONNECTED) return true;
    if (storedSSID.isEmpty()) return false;

    Serial.println("[WiFi] Reconnecting...");
    WiFi.disconnect();
    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED &&
           (millis() - start) < WIFI_CONNECT_TIMEOUT) {
        delay(250);
    }

    bool connected = WiFi.status() == WL_CONNECTED;
    if (connected) {
        Serial.println("[WiFi] Reconnected.");
    } else {
        Serial.println("[WiFi] Reconnect failed.");
    }
    return connected;
}

bool syncNTP(const Settings& settings) {
    configTime((long)(settings.utcOffset * 3600), 0, NTP_SERVER);

    struct tm timeinfo;
    int retries = 0;
    while (!getLocalTime(&timeinfo) && retries < 10) {
        delay(500);
        retries++;
    }

    if (retries >= 10) {
        Serial.println("[NTP] Sync failed.");
        return false;
    }

    Serial.print("[NTP] Time: ");
    Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
    return true;
}

bool isConnected() {
    return WiFi.status() == WL_CONNECTED;
}

bool isConnecting() {
    if (!connectionPending) return false;
    if (WiFi.status() == WL_CONNECTED) {
        connectionPending = false;
        Serial.printf("[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
        return false;
    }
    if (millis() - connectionStartedMs >= WIFI_CONNECT_TIMEOUT) {
        connectionPending = false;
        Serial.println("[WiFi] Connection attempt timed out.");
        return false;
    }
    return true;
}

String getIP() {
    if (apMode) return WiFi.softAPIP().toString();
    return WiFi.localIP().toString();
}

bool isAPMode() {
    return apMode;
}

} // namespace WifiManager
