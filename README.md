# Prayer-Bot: Smart Islamic Personal Assistant (ESP32‑S3)

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[📄 **README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md) | [📜 **License**](LICENSE)

Welcome to **Prayer‑Bot**, a DIY, offline‑first digital companion that helps you manage daily prayers, track Hijri dates, stay focused with a Pomodoro timer, and monitor local weather—all from a single ESP32‑S3 desk clock.

---

## 🌟 Key Features

- **🕌 Automated Prayer Times & Azan** – Monthly schedules from Aladhan with a repeating I2S prayer alarm that stops on the touch sensor or a shake.
- **🌙 Hijri Calendar & Event Tracker** – Current Islamic date, Ramadan countdown, major holidays, and custom offset handling.
- **⏱️ Pomodoro Focus Timer** – 25-minute focus and 5-minute touch-started break by default. A single beep sounds when focus ends; touch the sensor to begin the 2–5 minute break, after which the next focus session starts automatically. The existing long break still follows the configured cycle count.
- **📈 Weekly Pomodoro History** – Dashboard graph of daily sessions started and completed focus minutes for the last seven days. Daily totals are persisted in LittleFS; only completed focus sessions count toward focus minutes.
- **🌤️ Live Weather & AQI** – Weather and humidity from OpenWeatherMap and air-quality data from Open-Meteo.
- **📱 Local Web Dashboard** – Configure location, prayer calculation, prohibited-time offsets, azan volume, weather key, and Pomodoro timings; view device/prayer/weather status and control screens, Pomodoro, zikir counting, and the active alarm from a phone or computer.
- **📳 Shake‑to‑Snooze** – MPU6050 accelerometer lets you dismiss or snooze alerts with a shake.
- **🕋 Dhikr Screen** – Five Arabic dhikr phrases rotate every 20 seconds, with a touch-to-count counter; the screen returns to the main screen after 100 seconds.
- **👆 Single Touch Navigation** – The GPIO4 touch sensor advances to the next screen on each touch.
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
| **MPU6050 / GY‑521 IMU (I2C)** | 6‑axis accelerometer & gyroscope for shake detection; shares the I2C bus with the OLED. |
| **TTP223B Capacitive Touch Sensor** | GPIO 4 advances to the next screen; it also counts Dhikr, starts a waiting Pomodoro break, and dismisses Azan. |
| **INMP441 I2S Microphone (optional)** | Future voice‑command extensions (currently unused).
| **Micro‑USB Power / 5 V Power Supply** | Powers the board and peripherals.
| **Optional: SD Card (Micro‑SD slot)** | Can be added for large log storage; the core uses LittleFS on internal flash.

---

## 🌐 Access the local dashboard

### First-time setup

1. Flash and power on the ESP32.
2. On your phone or computer, open Wi-Fi settings and connect to **PrayerBot-Setup** (password **12345678**).
3. In a browser on that same device, go to **http://192.168.4.1**. This is the ESP32 setup access point's fixed IP address.
4. Enter the Wi-Fi name (SSID) and password for either:
   - your **2.4 GHz home Wi-Fi router**, or
   - your **mobile phone's 2.4 GHz hotspot**.
5. Select **Connect to Wi-Fi**. Wait for **“Wi-Fi connected successfully”**, then select **“Next: open dashboard”**. The setup access point stays available during this connection and the dashboard continues to work at **http://192.168.4.1** while your phone/computer remains connected to **PrayerBot-Setup**.
6. Set the location (Dhaka is the default) and other device settings from the dashboard.

### Open the dashboard later through your router or hotspot

After the ESP32 joins your router or phone hotspot, it receives a **local IP address from that network using DHCP**. This address is not fixed and may change. Find the **Home network IP** in the dashboard, or check the router/hotspot's connected-device list or the ESP32 serial log. Then connect your phone/computer to the **same router or hotspot** and open **http://<ESP32-home-IP>** in its browser (for example, `http://192.168.1.42`).

If you use a mobile hotspot, its owner phone provides the Wi-Fi connection to the ESP32. Some phones do not allow the hotspot-owner phone to reach connected clients; if the dashboard will not open on that phone, connect a second phone or computer to the hotspot and use the ESP32's displayed Home network IP there.

The dashboard exposes device status, today's prayer times, Hijri date, weather/AQI, screen selection, Pomodoro controls, zikir count controls, and the active azan stop control. Settings are range-validated and saved to LittleFS. The optional OpenWeather API key is never returned by the settings API; the dashboard only reports whether one is configured.

## ⏱️ Pomodoro flow

- Default focus is **25 minutes**; the touch-started break delay defaults to **5 minutes** and can be set from **2 to 5 minutes**.
- At the end of a focus session, the speaker plays one beep and the timer waits for the touch sensor (or the dashboard's Start/Pause control) before starting the break countdown.
- The next focus session starts automatically after the break. A configured long break is still used after the selected number of focus sessions.
- The dashboard's seven-day chart reports sessions started, completed focus sessions, and completed focus minutes. Statistics need valid device local time; partial or interrupted focus sessions are not included in completed-focus totals.

---

## 📚 Under the Hood (Software & APIs)

- **FreeRTOS** – Separate tasks for networking, UI rendering, sensor reading, and audio playback.
- **JSON handling** – Large API responses are parsed from network streams with ArduinoJson filters; web request bodies are size-limited and validated before settings are applied. JSON documents and Arduino `String` values use dynamic memory, so this is bounded in key paths rather than allocation-free.
- **Persistent storage and caching** – Settings, prayer/weather cache data, and Pomodoro daily history are stored in LittleFS. Cache metadata tracks expiry for supported cached data.
- **Free APIs** – Aladhan (prayer times) and Open‑Meteo (air quality); OpenWeatherMap weather requires a user-provided API key.

Feel free to fork, modify, and extend the bot (e.g., add voice commands or additional sensors). Happy building!

## 🛠️ Development Setup (VS Code + PlatformIO)

1. **Install VS Code** – download from https://code.visualstudio.com/ and run the installer.
2. **Add the PlatformIO extension** – open VS Code, go to Extensions (Ctrl+Shift+X), search for **PlatformIO IDE** and install it.
3. **Open the project** – choose **File → Open Folder…** and select the `e:\Prayer-Bot` directory.
4. **Build the firmware** – press the **PlatformIO: Build** button (or run `pio run` in the integrated terminal).
5. **Upload to the ESP32‑S3** – press **PlatformIO: Upload** (or `pio run --target upload`). Ensure the board is connected via USB.
6. **Serial monitor** – open **PlatformIO: Serial Monitor** (or `pio device monitor -b 115200`) to view boot logs and interact with the device.

> The PlatformIO tasks automatically use the configuration from `platformio.ini`.

### Arduino IDE

The project uses the same pinned dependency baseline with PlatformIO and Arduino IDE. Arduino does not provide an official LTS designation for these packages; use these tested, fixed versions rather than allowing automatic upgrades:

- **ESP32 by Espressif Systems** board package: **2.0.17**
- **ESPAsyncWebServer**: **3.6.0** (mathieucarbou)
- **AsyncTCP**: **3.3.2** (mathieucarbou)
- **U8g2**: **2.36.18**
- **Adafruit MPU6050**: **2.2.9**
- **Adafruit Unified Sensor**: **1.1.15**
- **Adafruit BusIO**: **1.17.4**
- **ESP8266Audio**: **2.2.0**
- **ArduinoJson**: **7.4.3**

In Arduino IDE, install the ESP32 board package at version **2.0.17** through Boards Manager and install the listed library versions. Remove or rename older duplicate ESPAsyncWebServer and AsyncTCP copies from your Arduino `libraries` folder so the IDE does not select them instead. Open `Prayer-Bot.ino` from the project root and select **ESP32S3 Dev Module**.

PlatformIO pins `espressif32@7.1.3`, which supplies the same Arduino-ESP32 **2.0.17** core, and the exact library versions above. Avoid upgrading the core or libraries independently in either environment unless you also test and update both builds.

---

## 📌 Pin Diagram

| Function | Pin (label) | GPIO |
|----------|-------------|------|
| OLED SDA / MPU6050 SDA (I²C data) | **SDA** | 11 |
| OLED SCL / MPU6050 SCL (I²C clock) | **SCL** | 12 |
| Capacitive Touch Sensor (screen advance) | **SIG** | 4 |
| I²S Amplifier – Bit Clock (BCLK) | **BCLK** | 5 |
| I²S Amplifier – Word Select / LRCK | **LRC/WS** | 6 |
| I²S Amplifier – Data In (DIN) | **DIN** | 7 |
| I²S Microphone – Serial Clock (SCK) | **MIC_SCK** | 16 |
| I²S Microphone – Word Select (WS) | **MIC_WS** | 17 |
| I²S Microphone – Data (SD) | **MIC_SD** | 18 |

These pins are defined in `src/config.h` and match the wiring shown in **HARDWARE_CONNECTIONS.md**.
