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

    const char DASHBOARD_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Prayer-Bot Dashboard</title>
    <style>
        :root {
            --bg: #0f172a; --card: #1e293b; --text: #f8fafc; --accent: #38bdf8;
            --border: #334155; --success: #22c55e;
        }
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: system-ui, -apple-system, sans-serif;
            background: var(--bg); color: var(--text);
            padding: 20px; line-height: 1.5;
        }
        .container { max-width: 800px; margin: 0 auto; }
        h1 { text-align: center; color: var(--accent); margin-bottom: 30px; font-size: 2rem; }
        .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(300px, 1fr)); gap: 20px; }
        .card {
            background: var(--card); border: 1px solid var(--border);
            border-radius: 12px; padding: 24px; box-shadow: 0 4px 6px -1px rgba(0,0,0,0.1);
        }
        .card h2 {
            color: var(--accent); font-size: 1.25rem; margin-bottom: 16px;
            padding-bottom: 8px; border-bottom: 1px solid var(--border);
        }
        .form-group { margin-bottom: 16px; }
        label { display: block; font-size: 0.875rem; color: #94a3b8; margin-bottom: 6px; }
        input, select {
            width: 100%; padding: 10px 12px; border: 1px solid var(--border);
            border-radius: 6px; background: #0f172a; color: var(--text); font-size: 1rem;
        }
        input[type="range"] { padding: 0; }
        .toggle-group { display: flex; align-items: center; gap: 12px; margin-bottom: 16px; }
        .toggle-group input[type="checkbox"] { width: 20px; height: 20px; accent-color: var(--accent); }
        .toggle-group label { margin-bottom: 0; font-size: 1rem; color: var(--text); }
        button {
            width: 100%; padding: 14px; border: none; border-radius: 8px;
            background: var(--accent); color: #0f172a; font-weight: 600; font-size: 1.1rem;
            cursor: pointer; margin-top: 30px; transition: opacity 0.2s;
        }
        button:hover { opacity: 0.9; }
        .status { text-align: center; color: var(--success); margin-top: 12px; font-weight: 500; height: 24px; }
        .hidden { display: none; }
    </style>
</head>
<body>
    <div class="container">
        <h1>&#x1F54C; Prayer-Bot Dashboard</h1>
        <div class="grid">
            
            <div class="card">
                <h2>&#x1F30D; Location Setup</h2>
                <div class="form-group">
                    <label>Country</label>
                    <select id="country" onchange="updateDivisions()">
                        <option value="BD">Bangladesh</option>
                        <option value="CUSTOM">Custom Location</option>
                    </select>
                </div>
                <div class="form-group" id="division-group">
                    <label>Division</label>
                    <select id="division" onchange="applyLocation()">
                        <option value="23.8103,90.4125">Dhaka</option>
                        <option value="22.3569,91.7832">Chattogram</option>
                        <option value="24.8949,91.8687">Sylhet</option>
                        <option value="24.3745,88.6042">Rajshahi</option>
                        <option value="22.8456,89.5403">Khulna</option>
                        <option value="22.7010,90.3535">Barishal</option>
                        <option value="25.7439,89.2752">Rangpur</option>
                        <option value="24.7471,90.4203">Mymensingh</option>
                    </select>
                </div>
                <div id="custom-group" class="hidden">
                    <div class="form-group">
                        <label>City Name</label>
                        <input type="text" id="city" value="Dhaka">
                    </div>
                    <div class="form-group" style="display:flex; gap:10px;">
                        <div style="flex:1;"><label>Lat</label><input type="number" id="lat" step="0.0001"></div>
                        <div style="flex:1;"><label>Lon</label><input type="number" id="lon" step="0.0001"></div>
                    </div>
                </div>
            </div>

            <div class="card">
                <h2>&#x1F50A; Audio & Azan</h2>
                <div class="toggle-group">
                    <input type="checkbox" id="azanOn">
                    <label for="azanOn">Enable Auto Azan</label>
                </div>
                <div class="form-group">
                    <label>Volume</label>
                    <input type="range" id="volume" min="0" max="100" value="80">
                </div>
                <div class="form-group">
                    <label>Hijri Day Offset (-2 to +2)</label>
                    <input type="number" id="hijriOffset" min="-2" max="2" value="0">
                </div>
            </div>

            <div class="card">
                <h2>&#x1F4DA; Prayer Method</h2>
                <div class="form-group">
                    <label>Calculation Method</label>
                    <select id="calcMethod">
                        <option value="1">Univ. of Islamic Sciences, Karachi</option>
                        <option value="2">Islamic Society of North America</option>
                        <option value="3">Muslim World League</option>
                        <option value="4">Umm Al-Qura University</option>
                        <option value="5">Egyptian General Authority</option>
                    </select>
                </div>
                <div class="form-group">
                    <label>Asr School</label>
                    <select id="asrSchool">
                        <option value="0">Shafi'i / Standard</option>
                        <option value="1">Hanafi</option>
                    </select>
                </div>
            </div>

            <div class="card">
                <h2>&#x23F1;&#xFE0F; Pomodoro Timer</h2>
                <div class="form-group">
                    <label>Focus Session (minutes)</label>
                    <input type="number" id="focusMin" value="25" min="1" max="120">
                </div>
                <div class="form-group" style="display:flex; gap:10px;">
                    <div style="flex:1;"><label>Short Break</label><input type="number" id="shortBrk" value="5" min="1" max="30"></div>
                    <div style="flex:1;"><label>Long Break</label><input type="number" id="longBrk" value="15" min="1" max="60"></div>
                </div>
                <div class="form-group">
                    <label>Cycles before Long Break</label>
                    <input type="number" id="pomCycles" value="4" min="1" max="10">
                </div>
            </div>

        </div>

        <button onclick="saveSettings()">Save & Apply Settings</button>
        <div class="status" id="status"></div>
    </div>

    <script>
        function updateDivisions() {
            const country = document.getElementById('country').value;
            if (country === 'BD') {
                document.getElementById('division-group').classList.remove('hidden');
                document.getElementById('custom-group').classList.add('hidden');
                applyLocation();
            } else {
                document.getElementById('division-group').classList.add('hidden');
                document.getElementById('custom-group').classList.remove('hidden');
            }
        }

        function applyLocation() {
            const sel = document.getElementById('division');
            const [lat, lon] = sel.value.split(',');
            document.getElementById('city').value = sel.options[sel.selectedIndex].text;
            document.getElementById('lat').value = lat;
            document.getElementById('lon').value = lon;
        }

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
                
                // Try to match division dropdown
                let matched = false;
                const divOpts = document.getElementById('division').options;
                for (let i = 0; i < divOpts.length; i++) {
                    if (divOpts[i].text === s.city) {
                        document.getElementById('country').value = 'BD';
                        document.getElementById('division').selectedIndex = i;
                        matched = true;
                        break;
                    }
                }
                if (!matched) document.getElementById('country').value = 'CUSTOM';
                updateDivisions();

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
                    const status = document.getElementById('status');
                    status.textContent = 'Settings saved successfully! Board is updating...';
                    setTimeout(() => status.textContent = '', 4000);
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
