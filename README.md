# Prayer-Bot: Smart Islamic Personal Assistant (ESP32‑S3)

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[📄 **README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md) | [📜 **License**](LICENSE)

Welcome to **Prayer‑Bot**, a DIY, offline‑first digital companion that helps you manage daily prayers, track Hijri dates, stay focused with a Pomodoro timer, and monitor local weather—all from a single ESP32‑S3 desk clock.

---

## 🌟 Key Features

- **🕌 Automated Prayer Times & Azan** – Monthly schedules from Aladhan with a repeating I2S prayer alarm that stops on either touch sensor or a shake.
- **🌙 Hijri Calendar & Event Tracker** – Current Islamic date, Ramadan countdown, major holidays, and custom offset handling.
- **⏱️ Pomodoro Focus Timer** – 25-minute focus and 5-minute touch-started break by default. A single beep sounds when focus ends; touch a sensor to begin the 2–5 minute break, after which the next focus session starts automatically. The existing long break still follows the configured cycle count.
- **📈 Weekly Pomodoro History** – Dashboard graph of daily sessions started and completed focus minutes for the last seven days. Daily totals are persisted in LittleFS; only completed focus sessions count toward focus minutes.
- **🌤️ Live Weather & AQI** – Weather and humidity from OpenWeatherMap and air-quality data from Open-Meteo.
- **📱 Local Web Dashboard** – Configure location, prayer calculation, prohibited-time offsets, azan volume, weather key, and Pomodoro timings; view device/prayer/weather status and control screens, Pomodoro, zikir counting, and the active alarm from a phone or computer.
- **📳 Shake‑to‑Snooze** – Vibration sensor lets you dismiss or snooze alerts with a shake.
- **🕋 Dhikr Screen** – Five Arabic dhikr phrases rotate every 20 seconds, with a touch-to-count counter; the screen returns to the main screen after 100 seconds.
- **🧩 Modular Firmware** – Features are separated into managers and display screens, with shared configuration and data types.

---

## 🗂️ Project structure

Firmware code lives in `src/`. Managers keep feature logic separate from the entry point, and each manager currently uses a matching `.h` interface and `.cpp` implementation. Screen rendering is grouped in `src/screens/`.

```text
Prayer-Bot/
├── platformio.ini             # PlatformIO board, framework, and dependencies
├── README.md                   # Project overview and setup
├── HARDWARE_CONNECTIONS.md    # Wiring guide
├── CIRCUIT.md                  # Circuit reference
├── data/                       # LittleFS image input (runtime files are generated)
└── src/
    ├── main.cpp                # Startup and FreeRTOS task orchestration
    ├── config.h                # Pins, defaults, API endpoints, and intervals
    ├── types.h                 # Shared application data types and states
    ├── *_manager.h/.cpp        # Prayer, weather, Wi-Fi, audio, storage, and other services
    ├── dashboard_html.h        # Embedded local web dashboard
    ├── pomodoro_stats.h/.cpp   # Persistent Pomodoro history and weekly report
    └── screens/                # OLED screen implementations and shared screen routing
```

**Where to make changes**

| Feature or concern | Main files |
|---|---|
| Prayer schedules and prohibited-time windows | `src/prayer_manager.h`, `src/prayer_manager.cpp` |
| Azan and Pomodoro notification sounds | `src/azan_manager.h`, `src/azan_manager.cpp` |
| Pomodoro state and durations | `src/pomodoro_manager.h`, `src/pomodoro_manager.cpp` |
| Pomodoro weekly totals | `src/pomodoro_stats.h`, `src/pomodoro_stats.cpp` |
| Dashboard UI and its JSON API | `src/dashboard_html.h`, `src/web_server_manager.h`, `src/web_server_manager.cpp` |
| Persistent settings and cache | `src/storage_manager.h`, `src/storage_manager.cpp`, `src/cache_manager.h`, `src/cache_manager.cpp` |
| OLED screen presentation | `src/screens/` and `src/screen_router.h`, `src/screen_router.cpp` |
| Shared settings, screen IDs, and defaults | `src/types.h`, `src/config.h` |
| Startup, task scheduling, and integration | `src/main.cpp` |

Keep feature behavior in its manager or screen module where practical; use `main.cpp` to initialize services and coordinate their FreeRTOS tasks. Keep interfaces in the corresponding headers and update this map when files or responsibilities move.

---

## 🛠️ Hardware Parts

| Component | Description |
|-----------|-------------|
| **ESP32‑S3 Development Board** (N16R8) | 240 MHz dual‑core MCU with 8 MB PSRAM and 16 MB flash – handles Wi‑Fi, audio, and graphics.
| **1.3" OLED Display (I2C, SSD1306/SH1106)** | Crisp monochrome UI for time, prayer list, weather, and Dhikr screen.
| **MAX98357A I2S Audio Amplifier** + small speaker | Plays Azan audio (MP3/AAC) via I2S without CPU load.
| **SW‑420 Vibration/Shake Sensor** | Detects a physical shake to snooze alerts.
| **Push Button** | Navigates menus and controls the Pomodoro timer.
| **INMP441 I2S Microphone (optional)** | Future voice‑command extensions (currently unused).
| **Micro‑USB Power / 5 V Power Supply** | Powers the board and peripherals.
| **Optional: SD Card (Micro‑SD slot)** | Can be added for large log storage; the core uses LittleFS on internal flash.

---

## 🌐 Access the local dashboard

1. Flash the firmware from VS Code/PlatformIO.
2. On first boot, join the device Wi-Fi network **PrayerBot-Setup** (password **12345678**).
3. Open **http://192.168.4.1**. This is the ESP32's default local access-point address.
4. The dashboard starts with **Dhaka** selected. Choose a preset city or enter a custom city name, latitude, longitude, timezone label, and UTC offset; then save your settings.
5. To give the ESP32 internet access for NTP, prayer schedules, and weather, expand **Wi-Fi setup / change network**, enter your home Wi-Fi credentials, and save. The board restarts. Open the dashboard at the ESP32's new LAN IP address (shown in the dashboard while connected by AP, in the serial log, or in your router's connected-device list).

The dashboard exposes device status, today's prayer times, Hijri date, weather/AQI, screen selection, Pomodoro controls, zikir count controls, and the active azan stop control. Settings are range-validated and saved to LittleFS. The optional OpenWeather API key is never returned by the settings API; the dashboard only reports whether one is configured.

## ⏱️ Pomodoro flow

- Default focus is **25 minutes**; the touch-started break delay defaults to **5 minutes** and can be set from **2 to 5 minutes**.
- At the end of a focus session, the speaker plays one beep and the timer waits for a touch sensor (or the dashboard's Start/Pause control) before starting the break countdown.
- The next focus session starts automatically after the break. A configured long break is still used after the selected number of focus sessions.
- The dashboard's seven-day chart reports sessions started, completed focus sessions, and completed focus minutes. Statistics need valid device local time; partial or interrupted focus sessions are not included in completed-focus totals.

---

## 📚 Under the Hood (Software & APIs)

- **FreeRTOS** – Separate tasks for networking, UI rendering, sensor reading, and audio playback.
- **JSON handling** – Large API responses are parsed from network streams with ArduinoJson filters; web request bodies are size-limited and validated before settings are applied. JSON documents and Arduino `String` values use dynamic memory, so this is bounded in key paths rather than allocation-free.
- **Persistent storage and caching** – Settings, prayer/weather cache data, and Pomodoro daily history are stored in LittleFS. Cache metadata tracks expiry for supported cached data.
- **Free APIs** – Aladhan (prayer times) and Open‑Meteo (air quality); OpenWeatherMap weather requires a user-provided API key.

Feel free to fork, modify, and extend the bot (e.g., add voice commands or additional sensors). Happy building!
