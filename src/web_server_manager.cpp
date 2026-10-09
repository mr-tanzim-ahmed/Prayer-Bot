#include "web_server_manager.h"
#include "config.h"
#include "storage_manager.h"
#include "prayer_manager.h"
#include "weather_manager.h"
#include "pomodoro_manager.h"
#include "calendar_manager.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>

// ============================================================
//  WEB SERVER MANAGER IMPLEMENTATION
//  Serves the phone dashboard and REST API.
// ============================================================

namespace {
    AsyncWebServer* server = nullptr;
    bool serverRunning = false;

    // Reference to settings (set during init)
    Settings* settingsRef = nullptr;
}

// Extern globals
extern WeatherData     g_weather;
extern DailyPrayers    g_todayPrayers;
extern NextPrayerInfo  g_nextPrayer;
extern HijriDate       g_hijriDate;
extern PomodoroState   g_pomodoro;

namespace {

    // Dashboard HTML (embedded)
    const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Prayer-Bot Dashboard</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', sans-serif;
            background: linear-gradient(135deg, #0c1220 0%, #1a2332 50%, #0d1b2a 100%);
            color: #e0e6ed;
            min-height: 100vh;
            padding: 16px;
        }
        h1 {
            text-align: center;
            color: #4fc3f7;
            margin-bottom: 20px;
            font-size: 1.4em;
        }
        .card {
            background: rgba(255,255,255,0.06);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 12px;
            padding: 16px;
            margin-bottom: 12px;
        }
        .card h2 {
            color: #81d4fa;
            font-size: 1em;
            margin-bottom: 12px;
            border-bottom: 1px solid rgba(255,255,255,0.1);
            padding-bottom: 6px;
        }
        label {
            display: block;
            font-size: 0.85em;
            color: #90a4ae;
            margin: 8px 0 4px;
        }
        input, select {
            width: 100%;
            padding: 8px 12px;
            border: 1px solid rgba(255,255,255,0.15);
            border-radius: 8px;
            background: rgba(0,0,0,0.3);
            color: #e0e6ed;
            font-size: 0.95em;
        }
        .toggle {
            display: flex;
            align-items: center;
            gap: 10px;
        }
        .toggle input[type="checkbox"] {
            width: 44px;
            height: 24px;
        }
        button {
            width: 100%;
            padding: 12px;
            border: none;
            border-radius: 8px;
            background: #4fc3f7;
            color: #0c1220;
            font-weight: bold;
            font-size: 1em;
            cursor: pointer;
            margin-top: 16px;
        }
        button:active { background: #29b6f6; }
        .status { text-align: center; color: #66bb6a; margin-top: 8px; font-size: 0.9em; }
    </style>
</head>
<body>
    <h1>&#x1F54C; Prayer-Bot</h1>

    <div class="card">
        <h2>Azan</h2>
        <div class="toggle">
            <input type="checkbox" id="azanOn" checked>
            <label for="azanOn" style="display:inline">Azan Enabled</label>
        </div>
        <label>Volume</label>
        <input type="range" id="volume" min="0" max="100" value="80">
    </div>

    <div class="card">
        <h2>Location</h2>
        <label>City</label>
        <input type="text" id="city" placeholder="Search city..." value="Dhaka">
        <label>Latitude</label>
        <input type="number" id="lat" step="0.0001">
        <label>Longitude</label>
        <input type="number" id="lon" step="0.0001">
    </div>

    <div class="card">
        <h2>Prayer Calculation</h2>
        <label>Method</label>
        <select id="calcMethod">
            <option value="1">University of Islamic Sciences, Karachi</option>
            <option value="2">Islamic Society of North America</option>
            <option value="3">Muslim World League</option>
            <option value="4">Umm Al-Qura University</option>
            <option value="5">Egyptian General Authority</option>
        </select>
        <label>Asr School</label>
        <select id="asrSchool">
            <option value="0">Shafi'i</option>
            <option value="1">Hanafi</option>
        </select>
        <label>Hijri Day Offset (-2 to +2)</label>
        <input type="number" id="hijriOffset" min="-2" max="2" value="0">
    </div>

    <div class="card">
        <h2>Pomodoro Timer</h2>
        <label>Focus (minutes)</label>
        <input type="number" id="focusMin" value="25" min="1" max="120">
        <label>Short Break (minutes)</label>
        <input type="number" id="shortBrk" value="5" min="1" max="30">
        <label>Long Break (minutes)</label>
        <input type="number" id="longBrk" value="15" min="1" max="60">
        <label>Cycles</label>
        <input type="number" id="pomCycles" value="4" min="1" max="10">
    </div>

    <button onclick="saveSettings()">Save Settings</button>
    <div class="status" id="status"></div>

    <script>
        async function loadSettings() {
            try {
                const r = await fetch('/api/settings');
                const s = await r.json();
                document.getElementById('azanOn').checked = s.azanOn;
                document.getElementById('volume').value = s.volume;
                document.getElementById('city').value = s.city;
                document.getElementById('lat').value = s.lat;
                document.getElementById('lon').value = s.lon;
                document.getElementById('calcMethod').value = s.calcMethod;
                document.getElementById('asrSchool').value = s.asrSchool;
                document.getElementById('hijriOffset').value = s.hijriOffset;
                document.getElementById('focusMin').value = s.focusMin;
                document.getElementById('shortBrk').value = s.shortBrk;
                document.getElementById('longBrk').value = s.longBrk;
                document.getElementById('pomCycles').value = s.pomCycles;
            } catch(e) { console.error(e); }
        }
        async function saveSettings() {
            const body = {
                azanOn: document.getElementById('azanOn').checked,
                volume: parseInt(document.getElementById('volume').value),
                city: document.getElementById('city').value,
                lat: parseFloat(document.getElementById('lat').value),
                lon: parseFloat(document.getElementById('lon').value),
                calcMethod: parseInt(document.getElementById('calcMethod').value),
                asrSchool: parseInt(document.getElementById('asrSchool').value),
                hijriOffset: parseInt(document.getElementById('hijriOffset').value),
                focusMin: parseInt(document.getElementById('focusMin').value),
                shortBrk: parseInt(document.getElementById('shortBrk').value),
                longBrk: parseInt(document.getElementById('longBrk').value),
                pomCycles: parseInt(document.getElementById('pomCycles').value)
            };
            try {
                const r = await fetch('/api/settings', {
                    method: 'POST',
                    headers: {'Content-Type': 'application/json'},
                    body: JSON.stringify(body)
                });
                if (r.ok) {
                    document.getElementById('status').textContent = 'Settings saved!';
                    setTimeout(() => document.getElementById('status').textContent = '', 3000);
                }
            } catch(e) { console.error(e); }
        }
        loadSettings();
    </script>
</body>
</html>
)rawliteral";
}

namespace WebServerManager {

void init(Settings& settings) {
    if (server) {
        delete server;
    }

    settingsRef = &settings;
    server = new AsyncWebServer(WEB_SERVER_PORT);

    // --- Dashboard page ---
    server->on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "text/html", DASHBOARD_HTML);
    });

    // --- GET settings ---
    server->on("/api/settings", HTTP_GET, [](AsyncWebServerRequest* request) {
        JsonDocument doc;
        doc["azanOn"]      = settingsRef->azanEnabled;
        doc["volume"]      = settingsRef->volume;
        doc["city"]        = settingsRef->cityName;
        doc["lat"]         = settingsRef->latitude;
        doc["lon"]         = settingsRef->longitude;
        doc["calcMethod"]  = settingsRef->calcMethod;
        doc["asrSchool"]   = settingsRef->asrSchool;
        doc["hijriOffset"] = settingsRef->hijriOffset;
        doc["focusMin"]    = settingsRef->focusMinutes;
        doc["shortBrk"]    = settingsRef->shortBreakMinutes;
        doc["longBrk"]     = settingsRef->longBreakMinutes;
        doc["pomCycles"]   = settingsRef->pomodoroCycles;

        String json;
        serializeJson(doc, json);
        request->send(200, "application/json", json);
    });

    // --- POST settings ---
    server->on("/api/settings", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        NULL,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String body;
            for (size_t i = 0; i < len; i++) {
                body += (char)data[i];
            }

            JsonDocument doc;
            if (deserializeJson(doc, body)) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

            // Update settings
            settingsRef->azanEnabled = doc["azanOn"] | settingsRef->azanEnabled;
            settingsRef->volume      = doc["volume"] | settingsRef->volume;

            bool cityChanged = false;
            if (doc["city"].is<const char*>()) {
                String newCity = String((const char*)doc["city"]);
                if (newCity != String(settingsRef->cityName)) {
                    strlcpy(settingsRef->cityName, newCity.c_str(), sizeof(settingsRef->cityName));
                    cityChanged = true;
                }
            }

            settingsRef->latitude         = doc["lat"]         | settingsRef->latitude;
            settingsRef->longitude        = doc["lon"]         | settingsRef->longitude;
            settingsRef->calcMethod       = doc["calcMethod"]  | settingsRef->calcMethod;
            settingsRef->asrSchool        = doc["asrSchool"]   | settingsRef->asrSchool;
            settingsRef->hijriOffset      = doc["hijriOffset"] | settingsRef->hijriOffset;
            settingsRef->focusMinutes     = doc["focusMin"]    | settingsRef->focusMinutes;
            settingsRef->shortBreakMinutes = doc["shortBrk"]   | settingsRef->shortBreakMinutes;
            settingsRef->longBreakMinutes = doc["longBrk"]     | settingsRef->longBreakMinutes;
            settingsRef->pomodoroCycles   = doc["pomCycles"]   | settingsRef->pomodoroCycles;

            // Save to flash
            StorageManager::saveSettings(*settingsRef);

            // If city changed, trigger re-fetch of prayer times and weather
            if (cityChanged) {
                PrayerManager::refresh(*settingsRef);
                CalendarManager::setHijriOffset(settingsRef->hijriOffset);
                // Weather will refresh on next network cycle
                Serial.println("[WEB] City changed. Re-fetching data...");
            }

            request->send(200, "application/json", "{\"status\":\"ok\"}");
            Serial.println("[WEB] Settings saved.");
        }
    );

    // --- WiFi setup endpoint (for AP mode) ---
    server->on("/api/wifi", HTTP_POST,
        [](AsyncWebServerRequest* request) {},
        NULL,
        [](AsyncWebServerRequest* request, uint8_t* data, size_t len, size_t index, size_t total) {
            String body;
            for (size_t i = 0; i < len; i++) body += (char)data[i];

            JsonDocument doc;
            if (deserializeJson(doc, body)) {
                request->send(400, "application/json", "{\"error\":\"Invalid JSON\"}");
                return;
            }

            String ssid = String((const char*)(doc["ssid"] | ""));
            String pass = String((const char*)(doc["pass"] | ""));

            if (ssid.isEmpty()) {
                request->send(400, "application/json", "{\"error\":\"SSID required\"}");
                return;
            }

            // Save WiFi credentials
            StorageManager::writeFile("/wifi_ssid.txt", ssid);
            StorageManager::writeFile("/wifi_pass.txt", pass);

            request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Restarting...\"}");

            // Restart to connect with new credentials
            delay(1000);
            ESP.restart();
        }
    );

    server->begin();
    serverRunning = true;

    Serial.print("[WEB] Dashboard: http://");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());
}

void stop() {
    if (server) {
        server->end();
        serverRunning = false;
    }
}

bool isRunning() {
    return serverRunning;
}

} // namespace WebServerManager
