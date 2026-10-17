#include "wifi_manager.h"
#include "config.h"
#include "storage_manager.h"
#include <ArduinoJson.h>
#include <WiFi.h>

// ============================================================
//  WIFI MANAGER IMPLEMENTATION
// ============================================================

namespace {
    constexpr char WIFI_CREDENTIALS_PATH[] = "/wifi_credentials.json";
    constexpr char LEGACY_WIFI_SSID_PATH[] = "/wifi_ssid.txt";
    constexpr char LEGACY_WIFI_PASSWORD_PATH[] = "/wifi_pass.txt";
    bool apMode = false;
    bool connectionPending = false;
    unsigned long connectionStartedMs = 0;
    String storedSSID;
    String storedPassword;
    String setupAPSSID = AP_SSID;
    String setupAPPassword = AP_PASSWORD;
    String pendingSetupAPSSID;
    String pendingSetupAPPassword;
    bool setupAPSettingsLoaded = false;
    bool setupAPSettingsPending = false;

    void loadSetupAPSettings() {
        if (setupAPSettingsLoaded) return;
        setupAPSettingsLoaded = true;
        if (!StorageManager::exists("/setup_ap.json")) return;

        JsonDocument doc;
        String content = StorageManager::readFile("/setup_ap.json");
        if (deserializeJson(doc, content) ||
            !doc["ssid"].is<String>() || !doc["pass"].is<String>()) {
            Serial.println("[WiFi] Invalid saved setup access point settings; using defaults.");
            return;
        }

        String ssid = doc["ssid"].as<String>();
        String password = doc["pass"].as<String>();
        if (ssid.isEmpty() || ssid.length() > 32 ||
            password.length() < 8 || password.length() > 63) {
            Serial.println("[WiFi] Saved setup access point settings are out of range; using defaults.");
            return;
        }

        setupAPSSID = ssid;
        setupAPPassword = password;
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
            if (ssid.isEmpty() || ssid.length() > 32 || pass.length() > 63) {
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
        Serial.println("[WiFi] No saved credentials. Starting AP...");
        startAP();
        return false;
    }

    storedSSID = ssid;
    storedPassword = pass;

    startAP();
    WiFi.setSleep(false);
    if (WiFi.begin(ssid.c_str(), pass.c_str()) == WL_CONNECT_FAILED) {
        Serial.println("[WiFi] Could not start the saved network connection.");
        return false;
    }

    Serial.print("[WiFi] Connecting to ");
    Serial.println(ssid);
    connectionStartedMs = millis();
    connectionPending = true;
    return WiFi.status() == WL_CONNECTED;
}

void startAP() {
    loadSetupAPSettings();
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    apMode = WiFi.softAP(setupAPSSID.c_str(), setupAPPassword.c_str());
    if (!apMode) {
        Serial.println("[WiFi] Failed to start setup access point.");
        return;
    }

    Serial.print("[WiFi] AP started: ");
    Serial.println(setupAPSSID);
    Serial.print("[WiFi] AP IP: ");
    Serial.println(WiFi.softAPIP());
}

String getSetupAPSSID() {
    loadSetupAPSettings();
    return setupAPSettingsPending ? pendingSetupAPSSID : setupAPSSID;
}

bool saveSetupAPSettings(const String& ssid, const String& password) {
    loadSetupAPSettings();
    if (ssid.isEmpty() || ssid.length() > 32 ||
        (!password.isEmpty() && (password.length() < 8 || password.length() > 63))) {
        return false;
    }

    const String selectedPassword = password.isEmpty()
        ? (setupAPSettingsPending ? pendingSetupAPPassword : setupAPPassword)
        : password;
    JsonDocument doc;
    doc["ssid"] = ssid;
    doc["pass"] = selectedPassword;
    String content;
    if (serializeJson(doc, content) == 0 || content.isEmpty()) {
        Serial.println("[WiFi] Could not serialize setup AP settings.");
        return false;
    }
    if (!StorageManager::writeFile("/setup_ap.json", content)) return false;

    pendingSetupAPSSID = ssid;
    pendingSetupAPPassword = selectedPassword;
    setupAPSettingsPending = true;
    return true;
}

bool saveCredentials(const String& ssid, const String& password) {
    if (ssid.isEmpty() || ssid.length() > 32 || password.length() > 63) {
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
    if (!StorageManager::removeFile(LEGACY_WIFI_SSID_PATH) ||
        !StorageManager::removeFile(LEGACY_WIFI_PASSWORD_PATH)) {
        Serial.println("[WiFi] New credentials were saved, but legacy credential files could not be removed.");
    }
    return true;
}

void applyPendingSetupAPSettings() {
    if (!setupAPSettingsPending) return;

    String previousSSID = setupAPSSID;
    String previousPassword = setupAPPassword;
    setupAPSSID = pendingSetupAPSSID;
    setupAPPassword = pendingSetupAPPassword;
    setupAPSettingsPending = false;

    WiFi.softAPdisconnect(false);
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    WiFi.setSleep(false);
    apMode = WiFi.softAP(setupAPSSID.c_str(), setupAPPassword.c_str());
    if (!apMode) {
        Serial.println("[WiFi] Failed to apply new setup access point settings; restoring previous settings.");
        setupAPSSID = previousSSID;
        setupAPPassword = previousPassword;
        apMode = WiFi.softAP(setupAPSSID.c_str(), setupAPPassword.c_str());
        JsonDocument doc;
        doc["ssid"] = previousSSID;
        doc["pass"] = previousPassword;
        String content;
        serializeJson(doc, content);
        if (!StorageManager::writeFile("/setup_ap.json", content)) {
            Serial.println("[WiFi] Failed to restore previous setup AP settings in storage.");
        }
    }
    if (apMode) {
        Serial.print("[WiFi] Setup AP active: ");
        Serial.println(setupAPSSID);
        Serial.print("[WiFi] AP IP: ");
        Serial.println(WiFi.softAPIP());
    }
}

bool connectToNetwork(const String& ssid, const String& password) {
    if (ssid.isEmpty() || ssid.length() > 32 || password.length() > 63) return false;
    storedSSID = ssid;
    storedPassword = password;

    WiFi.mode(apMode ? WIFI_AP_STA : WIFI_STA);
    WiFi.setSleep(false);
    if (WiFi.begin(storedSSID.c_str(), storedPassword.c_str()) == WL_CONNECT_FAILED) {
        Serial.println("[WiFi] Could not start the configured network connection.");
        return false;
    }
    connectionStartedMs = millis();
    connectionPending = true;
    Serial.printf("[WiFi] Connecting to configured network: %s\n", storedSSID.c_str());
    return true;
}

bool reconnect() {
    if (WiFi.status() == WL_CONNECTED) return true;
    if (storedSSID.isEmpty()) return false;

    if (connectionPending && millis() - connectionStartedMs < WIFI_CONNECT_TIMEOUT) {
        return false;
    }
    connectionPending = false;
    Serial.println("[WiFi] Reconnecting...");
    WiFi.disconnect();
    if (WiFi.begin(storedSSID.c_str(), storedPassword.c_str()) == WL_CONNECT_FAILED) {
        Serial.println("[WiFi] Reconnect attempt could not start.");
        return false;
    }
    connectionStartedMs = millis();
    connectionPending = true;
    return WiFi.status() == WL_CONNECTED;
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
