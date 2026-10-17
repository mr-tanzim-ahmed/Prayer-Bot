# Prayer-Bot: Smart Islamic Personal Assistant (ESP32‑S3)

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[📄 **README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md) | [📜 **License**](LICENSE)

Welcome to **Prayer‑Bot**, a DIY, offline‑first digital companion that helps you manage daily prayers, track Hijri dates, stay focused with a Pomodoro timer, and monitor local weather—all from a single ESP32‑S3 desk clock.

---

## 🌟 Key Features

- **🕌 Automated Prayer Times & Azan alert** – Monthly schedules from Aladhan refresh when the Prayer Focus screen opens and every 30 minutes while online, with an OLED sync indicator. A repeating I2S beep alert stops on the touch sensor or a shake. Recorded Adhan audio is not included in this build.
- **🌙 Hijri Calendar & Event Tracker** – Current Hijri date alongside the local Gregorian date, Ramadan countdown, major holidays, and custom offset handling.
- **⏱️ Pomodoro Focus Timer** – On the Pomodoro screen, hold the touch sensor for 3 seconds or shake the device to start an idle timer or pause/resume an active timer. A short tap while paused resumes the timer and advances to the next screen; after focus, a short tap starts the break and advances screens. The OLED shows a phase indicator, remaining time, animated status, and session progress.
- **📈 Weekly Pomodoro History** – Dashboard graph of daily sessions started and completed focus minutes for the last seven days. Daily totals are persisted in LittleFS; only completed focus sessions count toward focus minutes.
- **🌤️ Live Weather & AQI** – Weather and humidity from OpenWeatherMap and air-quality data from Open-Meteo; the OLED refreshes when opened and every 30 minutes, with a rotating sync indicator.
- **🎧 Surah Voice Player** – With router Wi-Fi/internet available, the last OLED screen listens for a Surah name for three seconds and sends clean 16 kHz mono microphone audio to Gemini for English or Bangla recognition. Shake the device on the Surah screen to stop active playback. The dashboard provides animated English/Bangla voice input; it sends the browser's speech transcript to Gemini through the ESP32, keeping the saved key off the phone/browser. Choose phone or Prayer-Bot audio output; if the primary recitation CDN is unavailable, playback tries an alternate source.
- **📱 Local Web Dashboard** – Configure location, prayer calculation, prohibited-time offsets, azan volume, and Pomodoro timings; view device/prayer/weather status and control screens, Pomodoro, zikir counting, Surah playback, and the active alarm from a phone or computer.
- **📳 Shake Controls** – The MPU6050 dismisses an active Azan alert; on the Pomodoro screen it starts or pauses/resumes the timer, and on the Surah screen it stops active playback.
- **🕋 Dhikr Screen** – Five English transliterations with their English meanings rotate every 5 seconds; touch increments the counter. The screen returns to the main screen after 100 seconds.
- **👆 Single Touch Navigation** – The GPIO4 touch sensor advances to the next screen on each touch.
- **🖥️ Seven OLED screens** – Prayer Focus, all prayer times, clock/calendar, weather, Pomodoro, Dhikr, and Quran each show the correct `screen/7` indicator.
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
| Voice recognition & Al-Quran assistant | `src/voice_manager.h`, `src/voice_manager.cpp`, `src/screens/surah_screen.h/.cpp` |
| Online API keys and default configuration | `src/config.h`, `src/storage_manager.cpp`, `src/dashboard_html.h` |
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
| **MAX98357A I2S Audio Amplifier** + small speaker | Plays repeating prayer/Pomodoro beeps and streamed MP3 Quran recitation over I2S.
| **MPU6050 / GY‑521 IMU (I2C)** | 6‑axis accelerometer & gyroscope for shake detection; shares the I2C bus with the OLED. |
| **TTP223B Capacitive Touch Sensor** | GPIO 4 advances to the next screen; it also counts Dhikr, starts a waiting Pomodoro break, and dismisses Azan. |
| **INMP441 I2S Microphone (optional)** | Captures a three-second Surah voice-selection clip for Gemini recognition when configured. |
| **Micro‑USB Power / 5 V Power Supply** | Powers the board and peripherals.
| **Optional: SD Card (Micro‑SD slot)** | Can be added for large log storage; the core uses LittleFS on internal flash.

---

## 🔑 API Keys & Services Configuration

Prayer-Bot connects to cloud APIs for AI speech recognition, weather conditions, prayer times, and Quran audio streaming.

### Summary of APIs Used

| Service / Feature | Provider | Purpose | Where to Configure |
|---|---|---|---|
| **Al-Quran Voice Recognition** | [Google Gemini Flash AI](https://aistudio.google.com/) | Maps spoken Surah names (English & Bangla / বাংলা) to Surah 1–114 | `src/config.h` or Web Dashboard |
| **Live Weather & Air Quality** | [OpenWeatherMap](https://openweathermap.org/api) | Current temperature, humidity, and condition | `src/config.h` or Web Dashboard |
| **Prayer Times & Timetable** | [Aladhan API](https://aladhan.com/prayer-times-api) | Monthly accurate calculation for Fajr, Dhuhr, Asr, Maghrib, Isha | Automatic (Free, no key needed) |
| **Air Quality Index (AQI)** | [Open-Meteo](https://open-meteo.com/en/docs/air-quality-api) | European AQI & PM2.5 monitoring | Automatic (Free, no key needed) |
| **Al-Quran MP3 Audio Streaming** | [MP3Quran CDN](https://mp3quran.net/) | Primary: Maher Al-Muaiqly / Fallback: Mishary Rashid Alafasy | `src/config.h` (Pre-configured) |

---

### How to Change Keys

You can configure and change API keys in two different ways:

#### Option 1: Via the Web Dashboard (Recommended — No Reflashing Needed)
1. Connect your phone or computer to the **Prayer-Bot** Wi-Fi hotspot or open the device IP in your browser (e.g. `http://192.168.4.1` or `http://<device-ip>/`).
2. Expand the **Device settings** card at the bottom.
3. Scroll to **Online services & API keys**.
4. Enter your keys:
   - **Gemini Flash API key**: Paste your Google Gemini API key.
   - **OpenWeatherMap API key**: Paste your OpenWeatherMap API key.
5. Click **Save & apply settings**.
   - The settings are automatically validated, saved to `/settings.json` in the ESP32's LittleFS flash storage, and applied immediately without requiring a reboot.

#### Option 2: Directly in Source Code (`src/config.h`)
Open [`src/config.h`](src/config.h) in your editor and update the default constants:

```cpp
// ------------------------------------------------------------
//  Surah Count & Online API Keys
// ------------------------------------------------------------
#define TOTAL_SURAHS          114

// 1. Google Gemini Flash API Key (for Slide 7 voice recognition & dashboard voice assistant)
// Get your free key at: https://aistudio.google.com/
#define DEFAULT_GEMINI_API_KEY "YOUR_GOOGLE_API_KEY"

// 2. OpenWeatherMap API Key (for Slide 4 weather & humidity)
// Get your free key at: https://openweathermap.org/api
#define DEFAULT_OWM_API_KEY    "YOUR_OPENWEATHERMAP_API_KEY_HERE"
```

Then rebuild and flash the firmware to your ESP32-S3.

#### Option 3: Via HTTP REST API
You can also update keys programmatically by sending an HTTP POST request to `/api/settings`:

```bash
curl -X POST http://192.168.4.1/api/settings \
  -H "Content-Type: application/json" \
  -d '{
    "city": "Dhaka",
    "timezone": "Asia/Dhaka",
    "lat": 23.8103,
    "lon": 90.4125,
    "utcOff": 6,
    "calcMethod": 1,
    "asrSchool": 1,
    "hijriOffset": 0,
    "azanOn": true,
    "volume": 80,
    "focusMin": 25,
    "shortBrk": 5,
    "longBrk": 15,
    "pomCycles": 4,
    "sunriseOff": 15,
    "zawalOff": 6,
    "sunsetOff": 15,
    "geminiKey": "YOUR_GEMINI_API_KEY",
    "owmKey": "YOUR_OPENWEATHERMAP_API_KEY_HERE"
  }'
```

---

## 🌐 Access the local dashboard

### First-time setup

1. Flash and power on the ESP32.
2. On your phone or computer, open Wi-Fi settings and connect to the ESP32's **Prayer-Bot** setup hotspot (default password **12345678**).
3. In a browser on that same device, go to **http://192.168.4.1**. This is the ESP32 setup access point's fixed IP address.
4. Enter the Wi-Fi name (SSID) and password for the separate router network the ESP32 should join:
   - your **2.4 GHz home Wi-Fi router**, or
   - your **mobile phone's 2.4 GHz hotspot**.
5. Select **Connect to Wi-Fi**. The ESP32 waits up to 35 seconds for association and a valid DHCP address/gateway, verifies saved credentials by reading them back, and saves them only after the router lease is valid. It then automatically restarts and reconnects. Internet access is not required to save valid router credentials; online services will need working internet separately. It keeps the **Prayer-Bot** setup hotspot active. The dashboard remains available at **http://192.168.4.1** when your phone/computer is connected to the ESP32 hotspot; the router connection is separate. Selecting a scanned network fills its SSID; enter the router password manually because scans cannot read passwords.
6. Set the location (Dhaka is the default) and other device settings from the dashboard. Selecting a preset fills its coordinates and timezone; entering a custom city and leaving the city field looks it up automatically. Save the settings to refresh prayer times and weather and sync the selected city's clock offset. When the dashboard opens, it initializes the ESP32 clock from the connected phone/computer's current time, so the clock can be set even without internet. Once the ESP32 is online, NTP updates it to the current time and retries periodically if synchronization is unavailable. The displayed time is not derived from the firmware compile timestamp.

Internet-backed features initialize after the ESP32 connects to the router. The station is considered connected only when Wi-Fi reports association and both a DHCP local address and gateway are present. New credentials are saved to LittleFS only after this check and a read-back verification; failed attempts retain the previous credentials. The connection timeout is 35 seconds. Router connectivity does not guarantee internet access: prayer/weather services, speech recognition, and Quran streaming separately require internet. If Wi-Fi disconnects, the firmware attempts to reconnect automatically. The dashboard displays offline status and disables Quran voice/audio controls; Pomodoro and Dhikr remain available locally. Browser voice recognition offers English and Bangla (বাংলা); allow microphone access in the browser. The ESP32 microphone assistant supports English or Bangla and also requires a configured Gemini key on the device.

For dashboard voice playback, select the speech language, press **Start voice input**, allow microphone access, and say the Surah name. Browser speech recognition produces the transcript; Gemini on the ESP32 maps it to a Surah number, then playback starts on the selected phone or Prayer-Bot output. Phone playback uses the browser audio element and falls back to an alternate CDN if the primary stream fails. Prayer-Bot playback streams MP3 audio and also tries an alternate CDN if needed. The ESP32 screen shows a microphone animation while listening, a recognition animation while Gemini is working, and the selected Surah name during playback. Gemini must already be provisioned privately on the device. Never place the API key in dashboard JavaScript or commit it to the repository; rotate keys pasted into chat or logs.

### Open the dashboard later through your router or hotspot

After the ESP32 joins your router or phone hotspot, it receives a **local IP address from that network using DHCP**. This address is not fixed and may change. Find the **Home network IP** in the dashboard, or check the router/hotspot's connected-device list or the ESP32 serial log. Then connect your phone/computer to the **same router or hotspot** and open **http://<ESP32-home-IP>** in its browser (for example, `http://192.168.1.42`).

If you use a mobile hotspot, its owner phone provides the Wi-Fi connection to the ESP32. Some phones do not allow the hotspot-owner phone to reach connected clients; if the dashboard will not open on that phone, connect a second phone or computer to the hotspot and use the ESP32's displayed Home network IP there.

The header's **Modules** menu links to the overview, setup guide, prayer schedule, prohibited-time windows, clock/calendar screen controls, Pomodoro history, weather, Dhikr, Quran audio, Wi-Fi setup, and device settings. The setup guide gives the basic Wi-Fi, location, and touch steps. The dashboard exposes device status, today's prayer times, all three calculated prohibited-time windows, Hijri date, weather/AQI, screen selection, Pomodoro controls, Dhikr count controls, Quran voice/player controls, and the active azan stop control. Settings are range-validated, saved to LittleFS, and reloaded by the dashboard after saving; storage failures are reported without applying unsaved values. Service keys are not displayed or editable in the dashboard.

The Prayer-Bot setup network uses its default hotspot name, password, and fixed address `192.168.4.1`; these are not configurable in the dashboard. Router credentials are saved only after association, a valid DHCP address and gateway, and storage read-back verification; then the device restarts. Wi-Fi scanning can populate the router SSID but cannot reveal its password; enter the password manually. Browser voice input uses the browser's speech-recognition support and requires microphone permission. Revoke any API key pasted into chat or public logs and provision replacements privately; API keys are not hardcoded in source.

Prayer schedules are fetched as a monthly calendar from Aladhan after the device has internet and valid local time, using the configured location and calculation method. The default is Dhaka, Bangladesh (23.8103, 90.4125), Asia/Dhaka (UTC+6); the downloaded month is cached on the device. This is not a preloaded/offline Bangladesh timetable or CSV, so the first schedule fetch and later month refreshes require network access. The dashboard's prohibited-time panel and the Prayer Focus OLED screen use the device's configured offsets: after sunrise, before Dhuhr (Zawal), and before Maghrib. Confirm calculation settings and local guidance for your area.

## 🕋 Dhikr phrases

The Dhikr screen cycles through these phrases (transliterations and short meanings are included for reference):

| English phrase | Meaning |
|---|---|
| Subhan Allah | Glory be to Allah |
| Alhamdulillah | Praise be to Allah |
| La ilaha illallah | There is no deity except Allah |
| Allahu Akbar | Allah is the Greatest |
| Astaghfirullah | I seek Allah's forgiveness |

The Dhikr OLED screen uses English text only; it does not require an Arabic font or Arabic image assets.

## ⏱️ Pomodoro flow

- Default focus is **25 minutes**; the touch-started break delay defaults to **5 minutes** and can be set from **2 to 5 minutes**.
- On the Pomodoro OLED screen, hold the touch sensor for 3 seconds to start or pause/resume. While paused, a short tap resumes the timer and goes to the next screen; a short tap while a break is ready starts the break and goes to the next screen. The OLED's animated status and progress bar reflect the current phase.
- A shake also starts the timer from idle or pauses/resumes an active focus/break timer while the Pomodoro screen is open. A shake while a focus session is waiting for its break does not start the break.
- At the end of a focus session, the speaker plays one beep and the timer waits for the touch sensor (or the dashboard's Start/Pause control) before starting the break countdown.
- The next focus session starts automatically after the break. A configured long break is still used after the selected number of focus sessions.
- The dashboard's seven-day chart reports sessions started, completed focus sessions, and completed focus minutes. Statistics need valid device local time; partial or interrupted focus sessions are not included in completed-focus totals.

---

## 📚 Under the Hood (Software & APIs)

- **FreeRTOS** – Separate tasks for networking, UI rendering, sensor reading, and audio playback.
- **JSON handling** – Large API responses are parsed from network streams with ArduinoJson filters; web request bodies are size-limited and validated before settings are applied. JSON documents and Arduino `String` values use dynamic memory, so this is bounded in key paths rather than allocation-free.
- **Persistent storage and caching** – Settings, prayer/weather cache data, and Pomodoro daily history are stored in LittleFS. Writes use a verified temporary file and rename to avoid exposing partially written JSON; filesystem access is serialized across web and background tasks. Once the filesystem is marked initialized, a mount failure no longer triggers an automatic format that could erase saved data. Cache metadata tracks expiry for supported cached data.
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

For the **single-file Arduino IDE build**, open `Prayer-Bot-Main\Prayer-Bot-Main.ino` from the project folder. It contains the application code in one sketch; you do not need to add the project's `src` files separately. Do not open the root `Prayer-Bot.ino` for this workflow; that is the modular project entry point.

- Install **ESP32 by Espressif Systems 2.0.17** from Boards Manager.
- Install these libraries at the listed versions:
  - **ArduinoJson**: **7.4.3**
  - **U8g2**: **2.36.18**
  - **ESPAsyncWebServer**: **3.6.0** (mathieucarbou)
  - **AsyncTCP**: **3.3.2** (mathieucarbou)
- Select **ESP32S3 Dev Module** and compile/upload.

The single-file sketch writes its generated beeps directly through the ESP32 I2S driver, so it does not require ESP8266Audio. The modular project under `src` additionally needs Adafruit MPU6050 **2.2.9**, Adafruit Unified Sensor **1.1.15**, Adafruit BusIO **1.17.4**, and ESP8266Audio **2.2.0**. Use these tested versions rather than allowing automatic upgrades, and remove or rename older duplicate ESPAsyncWebServer and AsyncTCP copies from your Arduino `libraries` folder so the IDE does not select them instead.

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
