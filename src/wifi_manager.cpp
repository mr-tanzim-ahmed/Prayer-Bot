#include "wifi_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <ArduinoJson.h>
#include <DNSServer.h>
#include <WiFi.h>

// ============================================================
//  WIFI MANAGER IMPLEMENTATION
// ============================================================

namespace {
    constexpr char WIFI_CREDENTIALS_PATH[] = "/wifi_credentials.json";
    constexpr char LEGACY_WIFI_SSID_PATH[] = "/wifi_ssid.txt";
    constexpr char LEGACY_WIFI_PASSWORD_PATH[] = "/wifi_pass.txt";
    constexpr uint16_t DNS_PORT = 53;
    DNSServer captiveDnsServer;
    bool apMode = false;
    bool connectionPending = false;
    unsigned long connectionStartedMs = 0;
    String storedSSID;
    String storedPassword;
    String pendingSSID;
    String pendingPassword;
    bool pendingCredentialSave = false;
    bool restartWhenConnected = false;
    bool connectionAttemptFailed = false;

    bool stationHasRouterLease() {
        if (WiFi.status() != WL_CONNECTED) return false;

        const IPAddress localAddress = WiFi.localIP();
        const IPAddress gatewayAddress = WiFi.gatewayIP();
        const IPAddress zeroAddress(0, 0, 0, 0);
        return localAddress != zeroAddress &&
               gatewayAddress != zeroAddress;
    }
}

namespace WifiManager {

bool init(const Settings& settings) {
    (void)settings;
    String ssid;
    String pass;
    if (StorageManager::exists(WIFI_CREDENTIALS_PATH)) {
        JsonDocument doc;
        String content = StorageManager::readFile(WIFI_CREDENTIALS_PATH);
        if (!content.isEmpty() && !deserializeJson(doc, content) &&
            doc["ssid"].is<const char*>() && doc["pass"].is<const char*>()) {
            ssid = doc["ssid"].as<String>();
            pass = doc["pass"].as<String>();
            if (ssid.isEmpty() || ssid.length() > 32 ||
                (!pass.isEmpty() && pass.length() < 8) || pass.length() > 63) {
                Serial.println("[WiFi] Saved credentials are outside supported limits.");
                ssid = "";
                pass = "";
            }
        } else {
            Serial.println("[WiFi] Saved credentials are invalid; falling back to legacy credentials.");
        }
    }
    if (ssid.isEmpty()) {
        ssid = StorageManager::readFile(LEGACY_WIFI_SSID_PATH);
        pass = StorageManager::readFile(LEGACY_WIFI_PASSWORD_PATH);
        if (!ssid.isEmpty()) saveCredentials(ssid, pass);
    }
    if (ssid.length() > 32 || pass.length() > 63) {
        Serial.println("[WiFi] Saved network credentials exceed supported limits.");
        ssid = "";
        pass = "";
    }

    if (ssid.isEmpty()) {
#ifdef DEFAULT_STA_SSID
        if (strlen(DEFAULT_STA_SSID) > 0) {
            ssid = DEFAULT_STA_SSID;
            pass = DEFAULT_STA_PASSWORD;
            Serial.printf("[WiFi] No saved credentials; auto-connecting to default network: %s\n", ssid.c_str());
        }
#endif
    }

    if (ssid.isEmpty()) {
        Serial.println("[WiFi] No saved credentials. Starting AP...");
        startAP();
        return false;
    }

    storedSSID = ssid;
    storedPassword = pass;

    startAP();
    WiFi.setSleep(false);
    connectionAttemptFailed = false;
    connectionStartedMs = millis();
    connectionPending = true;
    if (WiFi.begin(ssid.c_str(), pass.c_str()) == WL_CONNECT_FAILED) {
        Serial.println("[WiFi] Could not start the saved network connection.");
        connectionPending = false;
        connectionAttemptFailed = true;
        return false;
    }

    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);
    return stationHasRouterLease();
}

void startAP() {
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    WiFi.setAutoReconnect(true);
    apMode = WiFi.softAP(AP_SSID, AP_PASSWORD);
    if (!apMode) {
        Serial.println("[WiFi] Failed to start setup access point.");
        return;
    }

    Serial.print("[WiFi] AP started: ");
    Serial.println(AP_SSID);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
    if (!captiveDnsServer.start(DNS_PORT, "*", WiFi.softAPIP())) {
        Serial.println("[WiFi] Could not start captive-portal DNS.");
    } else {
        Serial.println("[WiFi] Captive-portal DNS started.");
    }
}

void processCaptivePortal() {
    if (apMode) captiveDnsServer.processNextRequest();
}

String getSetupAPSSID() {
    return String(AP_SSID);
}

bool saveCredentials(const String& ssid, const String& password) {
    if (ssid.isEmpty() || ssid.length() > 32 ||
        (!password.isEmpty() && password.length() < 8) ||
        password.length() > 63) {
        return false;
    }
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["pass"] = password;
    String content;
    if (serializeJson(doc, content) == 0 || content.isEmpty()) {
        Serial.println("[WiFi] Could not serialize network credentials.");
        return false;
    }
    if (!StorageManager::writeFile(WIFI_CREDENTIALS_PATH, content)) return false;

    JsonDocument verification;
    const String savedContent = StorageManager::readFile(WIFI_CREDENTIALS_PATH);
    if (savedContent.isEmpty() ||
        deserializeJson(verification, savedContent)) {
        Serial.println("[WiFi] Saved credentials failed read-back verification.");
        return false;
    }
    const String savedSSID = verification["ssid"] | "";
    const String savedPassword = verification["pass"] | "";
    if (savedSSID != ssid || savedPassword != password) {
        Serial.println("[WiFi] Saved credentials failed read-back verification.");
        return false;
    }

    if (!StorageManager::removeFile(LEGACY_WIFI_SSID_PATH) ||
        !StorageManager::removeFile(LEGACY_WIFI_PASSWORD_PATH)) {
        Serial.println("[WiFi] New credentials were saved, but legacy credential files could not be removed.");
    }
    return true;
}

bool connectToNetwork(const String& ssid, const String& password) {
    if (ssid.isEmpty() || ssid.length() > 32 ||
        (!password.isEmpty() && password.length() < 8) ||
        password.length() > 63 || pendingCredentialSave) {
        if (pendingCredentialSave) {
            Serial.println("[WiFi] A new credential test is already in progress.");
        }
        return false;
    }

    WiFi.mode(apMode ? WIFI_AP_STA : WIFI_STA);
    WiFi.setSleep(false);
    connectionAttemptFailed = false;
    restartWhenConnected = false;
    connectionPending = false;
    pendingCredentialSave = false;
    pendingSSID = "";
    pendingPassword = "";
    WiFi.disconnect(false, false);
    delay(100);
    if (WiFi.begin(ssid.c_str(), password.c_str()) == WL_CONNECT_FAILED) {
        Serial.println("[WiFi] Could not start the configured network connection.");
        connectionAttemptFailed = true;
        return false;
    }
    pendingSSID = ssid;
    pendingPassword = password;
    pendingCredentialSave = true;
    connectionStartedMs = millis();
    connectionPending = true;
    Serial.printf("[WiFi] Connecting to configured network: %s\n", ssid.c_str());
    return true;
}

void restartAfterConnection() {
    restartWhenConnected = true;
}

bool shouldRestartAfterConnection() {
    return restartWhenConnected && stationHasRouterLease() &&
           !connectionPending && !pendingCredentialSave;
}

bool reconnect() {
    if (stationHasRouterLease()) return true;
    if (storedSSID.isEmpty()) {
#ifdef DEFAULT_STA_SSID
        if (strlen(DEFAULT_STA_SSID) > 0) {
            storedSSID = DEFAULT_STA_SSID;
            storedPassword = DEFAULT_STA_PASSWORD;
        } else {
            return false;
        }
#else
        return false;
#endif
    }

    if (connectionPending && millis() - connectionStartedMs < WIFI_CONNECT_TIMEOUT) {
        return false;
    }
    connectionPending = false;
    Serial.println("[WiFi] Reconnecting...");
    WiFi.disconnect();
    connectionAttemptFailed = false;
    connectionStartedMs = millis();
    connectionPending = true;
    if (WiFi.begin(storedSSID.c_str(), storedPassword.c_str()) == WL_CONNECT_FAILED) {
        Serial.println("[WiFi] Reconnect attempt could not start.");
        connectionPending = false;
        connectionAttemptFailed = true;
        return false;
    }
    return stationHasRouterLease();
}

bool syncNTP(const Settings& settings) {
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[NTP] WiFi is not connected; automatic time sync will retry later.");
        return false;
    }

    configTime((long)(settings.utcOffset * 3600), 0, NTP_SERVER);

    struct tm timeinfo;
    for (int retry = 0; retry < 10; ++retry) {
        if (getLocalTime(&timeinfo, 500) &&
            timeinfo.tm_year >= (2020 - 1900) &&
            timeinfo.tm_mon >= 0 && timeinfo.tm_mon < 12 &&
            timeinfo.tm_mday >= 1 && timeinfo.tm_mday <= 31) {
            Serial.print("[NTP] Device local date/time: ");
            Serial.println(&timeinfo, "%Y-%m-%d %H:%M:%S");
            return true;
        }
        delay(500);
    }

    Serial.println("[NTP] No valid date/time received; will retry automatically.");
    return false;
}

bool isConnected() {
    return stationHasRouterLease();
}

bool connectionInProgress() {
    return connectionPending;
}

void abortConnection() {
    if (connectionPending) {
        Serial.println("[WiFi] Aborting active connection attempt for scan.");
        connectionPending = false;
        pendingCredentialSave = false;
        pendingSSID = "";
        pendingPassword = "";
        restartWhenConnected = false;
        WiFi.disconnect(false, false);
    }
}

bool isConnecting() {
    if (!connectionPending) return false;
    if (stationHasRouterLease()) {
        if (pendingCredentialSave) {
            if (!saveCredentials(pendingSSID, pendingPassword)) {
                Serial.println("[WiFi] Connected, but could not save the new credentials; keeping previous credentials.");
                connectionAttemptFailed = true;
                pendingCredentialSave = false;
                pendingSSID = "";
                pendingPassword = "";
                restartWhenConnected = false;
                WiFi.disconnect(false, false);
                if (!storedSSID.isEmpty()) {
                    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());
                    connectionStartedMs = millis();
                    connectionPending = true;
                } else {
                    connectionPending = false;
                }
                return connectionPending;
            }
            storedSSID = pendingSSID;
            storedPassword = pendingPassword;
            pendingSSID = "";
            pendingPassword = "";
            pendingCredentialSave = false;
            Serial.println("[WiFi] Connected credentials saved and verified.");
        }
        connectionPending = false;
        Serial.printf("[WiFi] Connected. IP: %s\n", WiFi.localIP().toString().c_str());
        return false;
    }
    if (millis() - connectionStartedMs >= WIFI_CONNECT_TIMEOUT) {
        Serial.printf("[WiFi] Connection timed out (status=%d, local=%s, gateway=%s).\n",
                      static_cast<int>(WiFi.status()),
                      WiFi.localIP().toString().c_str(),
                      WiFi.gatewayIP().toString().c_str());
        connectionPending = false;
        pendingCredentialSave = false;
        pendingSSID = "";
        pendingPassword = "";
        restartWhenConnected = false;
        connectionAttemptFailed = true;
        Serial.println("[WiFi] Connection attempt timed out.");
        return false;
    }
    return true;
}

bool didConnectionAttemptFail() {
    return connectionAttemptFailed;
}

String getIP() {
    if (stationHasRouterLease()) return WiFi.localIP().toString();
    if (apMode) return WiFi.softAPIP().toString();
    return String();
}

bool isAPMode() {
    return apMode;
}

} // namespace WifiManager
