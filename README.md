# Prayer-Bot: Smart Islamic Personal Assistant (ESP32‑S3)

[![MIT License](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

[📄 **README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md) | [📜 **License**](LICENSE)

**Prayer‑Bot** is an open-source ESP32‑S3 prayer clock and personal dashboard for daily prayer times, Hijri dates, Quran listening, focus sessions, and local weather. Its OLED screens and web dashboard bring those everyday tools together in one DIY project.

---

## 🌟 Key Features

- **🕌 Automated Prayer Times & Azan alert** – Monthly schedules from Aladhan refresh when the Prayer Focus screen opens and every 30 minutes while online, with an OLED sync indicator. A repeating I2S beep alert stops on the touch sensor or a shake. Recorded Adhan audio is not included in this build.
- **🌙 Hijri Calendar & Event Tracker** – Current Hijri date alongside the local Gregorian date, Ramadan countdown, major holidays, and custom offset handling.
- **⏱️ Pomodoro Focus Timer** – On the Pomodoro screen, hold the touch sensor for 3 seconds or shake the device to start an idle timer or pause/resume an active timer. A short tap while paused resumes the timer and advances to the next screen; after focus, a short tap starts the break and advances screens. The OLED shows a phase indicator, remaining time, animated status, and session progress.
- **📈 Weekly Pomodoro History** – Dashboard graph of daily sessions started and completed focus minutes for the last seven days. Daily totals are persisted in LittleFS; only completed focus sessions count toward focus minutes.
- **🌤️ Live Weather & AQI** – The 1.3-inch OLED rotates through OpenWeatherMap air temperature, condition and humidity (3 seconds), its own 1–5 air-quality index and PM2.5 (2 seconds), then pressure and wind speed (3 seconds). Readings use the configured coordinates and refresh every 30 minutes; phone apps may differ because of their provider, location, update time, or use of feels-like temperature.
- **🎧 Quran Voice Assistant** – Ask by voice or text for a Surah in English or Bangla, or request the Prayer-Bot Azan notification tone. Gemini interprets dashboard requests through the ESP32; the saved key never enters the browser. Choose phone or device-speaker playback. Phone playback can use MP3Quran's timed recitations and show the current ayah; recorded Adhan audio is not included.
- **📱 Local Web Dashboard** – Configure location, prayer calculation, prohibited-time offsets, azan volume, and Pomodoro timings; view device/prayer/weather status and control screens, Pomodoro, zikir counting, Surah playback, and the active alarm from a phone or computer.
- **📳 Shake Controls** – The MPU6050 dismisses an active Azan alert; on the Pomodoro screen it starts or pauses/resumes the timer, and on the Surah screen it stops active playback.
- **🕋 Dhikr Screen** – Five English transliterations with their English meanings rotate every 5 seconds; touch increments the counter. The screen returns to the main screen after 100 seconds.
- **👆 Single Touch Navigation** – The GPIO4 touch sensor advances to the next screen on each touch with a quiet speaker beep.
- **🖥️ Seven OLED screens** – Prayer Focus, all prayer times, clock/calendar, weather, Pomodoro, Dhikr, and Quran use a consistent, compact 1.3-inch layout with clear spacing and no screen-number badges.
- **🧩 Modular Firmware** – Features are separated into managers and display screens, with shared configuration and data types.

---

## 🗂️ Project structure

Firmware code lives in `src/`. Managers keep feature logic separate from the entry point, and each manager currently uses a matching `.h` interface and `.cpp` implementation. Screen rendering is grouped in `src/screens/`.

```text
Prayer-Bot/
├── platformio.ini              # PlatformIO board, framework, and dependencies
├── README.md                    # Project overview and setup
├── HARDWARE_CONNECTIONS.md    # Wiring guide
├── CIRCUIT.md                   # Circuit overview and wiring reference
├── circuit_image.svg            # Circuit diagram illustration
├── data/                        # LittleFS image input (runtime files are generated)
└── src/
    ├── main.cpp                 # Startup, FreeRTOS tasks, and module coordination
    ├── config.h                 # Hardware pins, limits, endpoints, and defaults
    ├── secrets.h                # Local-only API keys and optional router defaults (ignored by Git)
    ├── types.h                  # Shared settings, state, and data models
    ├── *_manager.h/.cpp         # Feature services with narrow module interfaces
    ├── dashboard_html.h         # Embedded dashboard markup, styles, and browser logic
    └── screens/                 # OLED views and shared rendering helpers
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
| Private API-key defaults and online service configuration | `src/config.h`, `src/secrets.h`, `src/storage_manager.cpp` |
| OLED screen presentation | `src/screens/` and `src/screen_router.h`, `src/screen_router.cpp` |
| Shared settings, screen IDs, and defaults | `src/types.h`, `src/config.h` |
| Startup, task scheduling, and integration | `src/main.cpp` |

Keep feature behavior in its manager or screen module where practical; use `main.cpp` to initialize services and coordinate their FreeRTOS tasks. Keep interfaces in the corresponding headers and update this map when files or responsibilities move.

---

## 🛠️ Hardware Parts

| Component | Description |
|-----------|-------------|
| **ESP32‑S3 Development Board** (N16R8) | 240 MHz dual‑core MCU with 8 MB PSRAM and 16 MB flash – handles Wi‑Fi, audio, and graphics. PlatformIO uses the DevKitC-1 base profile with explicit N16R8 memory and flash overrides.
| **1.3" OLED Display (I2C, SSD1306/SH1106)** | Crisp monochrome UI for time, prayer list, weather, and Dhikr screen.
| **MAX98357A I2S Audio Amplifier** + small speaker | Plays repeating prayer/Pomodoro beeps and streamed MP3 Quran recitation over I2S.
| **MPU6050 / GY‑521 IMU (I2C)** | 6‑axis accelerometer & gyroscope for shake detection; shares the I2C bus with the OLED. |
| **TTP223B Capacitive Touch Sensor** | GPIO 4 advances to the next screen; it also counts Dhikr, starts a waiting Pomodoro break, and dismisses Azan. |
| **INMP441 I2S Microphone (optional)** | Captures a three-second Surah voice-selection clip for Gemini recognition when configured. |
| **Micro‑USB Power / 5 V Power Supply** | Powers the board and peripherals.
| **Optional: SD Card (Micro‑SD slot)** | Can be added for large log storage; the core uses LittleFS on internal flash.

---

## 🔑 ESP32-S3 API Key Setup

Gemini powers Surah-name recognition in the Quran assistant, while OpenWeatherMap provides weather and air-quality readings. Add those credentials to a private `src/secrets.h` file before building the firmware. Prayer times and Quran audio metadata do not need keys from you.

### Summary of APIs Used

| Service / Feature | Provider | Purpose | Where to Configure |
|---|---|---|---|
| **Quran Assistant** | [Google Gemini](https://aistudio.google.com/) | Identifies spoken or typed Surah requests and Azan-notification commands | Private `src/secrets.h` |
| **Live Weather & Air Quality** | [OpenWeatherMap](https://openweathermap.org/api) | Current temperature, humidity, condition, AQI (1–5), and pollutant concentrations | Private `src/secrets.h` |
| **Prayer Times & Timetable** | [Aladhan API](https://aladhan.com/prayer-times-api) | Monthly accurate calculation for Fajr, Dhuhr, Asr, Maghrib, Isha | Automatic (Free, no key needed) |
| **Air Quality Index (AQI)** | [OpenWeatherMap Air Pollution API](https://openweathermap.org/api/air-pollution) | AQI (1–5), PM2.5, PM10, and pollutant concentrations | Uses the OpenWeatherMap API key |
| **Device Quran Audio** | [MP3Quran v3 API](https://www.mp3quran.net/eng/api) and CDN | Reciter metadata selects the device's recitation; an alternate audio source is tried if needed | API metadata fetched automatically |
| **Phone Quran Audio & Ayah Timing** | [MP3Quran Timing API](https://www.mp3quran.net/eng/timing-api) | Select a timed reciter and sync the phone player's progress to the current ayah | Loaded automatically when the device is online |

The firmware uses the Timing API's `GET /ayat_timing/reads`, `GET /ayat_timing/soar?read=<id>`, and `GET /ayat_timing?surah=<number>&read=<id>` endpoints. Live validation confirmed the reads and chapter lists and returned millisecond ayah start/end ranges; the selected reciter's Surah MP3 URL was also checked. Timed ayah progress is for phone/browser playback only.

The dashboard assistant resolves a Surah name to its number and Arabic/API chapter entry, shows the matched Surah and playback state in the web UI, and starts the chosen phone or device-speaker output. It identifies Surahs; it does not provide Quran translation, tafsir, or free-form Quran-text search.

---

### 🔒 Configure private keys in `src/secrets.h`

`src/config.h` includes `src/secrets.h`, and the firmware reads the default Gemini and OpenWeatherMap keys from it during startup. The repository includes [`src/secrets.h.example`](src/secrets.h.example) as a safe template. Copy it to **`src/secrets.h`** on your computer, then edit that private copy with your own keys. The real `secrets.h` file is excluded by `.gitignore`; the `.example` template is tracked so it can be shared safely with the project.

#### Create your local secrets file

From the repository root, run:

```powershell
Copy-Item src\secrets.h.example src\secrets.h
```

Then open `src/secrets.h` and replace `"Your_gemini_key"` and `"Your_openweathermap_key"` with the keys issued by [Google AI Studio](https://aistudio.google.com/) and [OpenWeatherMap](https://openweathermap.org/api). Keep the quotation marks. These placeholder values are examples, not working credentials. Leave the router SSID and password empty if you plan to set up Wi-Fi from the device dashboard.

**If `src/secrets.h` already exists, do not run the copy command:** it would replace your local file. Keep your existing keys and edit only the values you intend to change. Do not paste a key into this README, `config.h`, dashboard code, screenshots, or public logs.

#### Setup checklist

1. Get a Gemini API key from [Google AI Studio](https://aistudio.google.com/) and an OpenWeatherMap API key from [OpenWeatherMap](https://openweathermap.org/api). Use the provider's current account and usage terms.
2. If `src/secrets.h` is missing, copy `src/secrets.h.example` to `src/secrets.h`. If it already exists, **do not overwrite it**—preserve its existing keys and change only the values you intend to update.
3. Replace the example API key strings with your own. Leave the router macros empty to join Wi-Fi using the setup dashboard, or fill them with local router credentials for first-boot auto-connect.
4. Build and upload the firmware using PlatformIO. The key defaults are compiled into the device; the dashboard does not display or edit them. See [Development Setup](#development-setup-vs-code-platformio) for the build and upload steps.
5. Check that Git is ignoring your local file before committing: run `git check-ignore src/secrets.h`. Do not force-add it or place credentials in any tracked file.

If a real key is accidentally committed or shared publicly, revoke it with its provider and create a replacement. The device's web dashboard and `/api/settings` endpoint do not expose or accept API keys.

> [!CAUTION]
> Treat API keys like passwords. Do not share them in chat, issue comments, commits, or screenshots. Rotate any key that may have been exposed.

---



### First-time setup

1. Flash and power on the ESP32.
2. On your phone or computer, open Wi-Fi settings and connect to the ESP32's **Prayer-Bot** setup hotspot (password **12345678**).
3. Accept the **Sign in to network / Captive portal** notification; the setup dashboard should open automatically. The firmware redirects common phone and computer connectivity checks to the setup page. If your device does not show a portal notification, open **http://192.168.4.1** while still connected to the Prayer-Bot hotspot.
4. Enter the Wi-Fi name (SSID) and password for the separate router network the ESP32 should join:
   - your **2.4 GHz home Wi-Fi router**, or
   - your **mobile phone's 2.4 GHz hotspot**.
5. Select **Connect to Wi-Fi**. The ESP32 waits up to 35 seconds for association and a valid DHCP address/gateway, verifies saved credentials by reading them back, and saves them only after the router lease is valid. It then automatically restarts and reconnects. Internet access is not required to save valid router credentials; online services will need working internet separately. The **Prayer-Bot** setup hotspot remains active. Selecting a scanned network fills its SSID; enter the router password manually because scans cannot read passwords.
6. Set the location (Dhaka is the default) and other device settings from the dashboard. Selecting a preset fills its coordinates and timezone; entering a custom city and leaving the city field looks it up automatically. Save the settings to refresh prayer times and weather and sync the selected city's clock offset. When the dashboard opens, it initializes the ESP32 clock from the connected phone/computer's current time, so the clock can be set even without internet. Once the ESP32 is online, NTP updates it to the current time and retries periodically if synchronization is unavailable. The displayed time is not derived from the firmware compile timestamp.

Internet-backed features initialize after the ESP32 connects to the router. The station is considered connected only when Wi-Fi reports association and both a DHCP local address and gateway are present. New credentials are saved to LittleFS only after this check and a read-back verification; failed attempts retain the previous credentials. The connection timeout is 35 seconds. Router connectivity does not guarantee internet access: prayer/weather services, speech recognition, and Quran streaming separately require internet. If Wi-Fi disconnects, the firmware attempts to reconnect automatically. The dashboard displays offline status and disables Quran voice/audio controls; Pomodoro and Dhikr remain available locally. Browser voice recognition offers English and Bangla (বাংলা); allow microphone access in the browser. Dashboard transcripts are sent to Gemini through the ESP32. The Gemini key is provisioned privately on the device and is never sent to the browser.

For dashboard voice playback, select the speech language, press **Speak**, allow microphone access, and ask for a Surah. You can also type a request. Browser speech recognition produces the voice transcript; Gemini on the ESP32 identifies the Surah or Azan notification request. For Surahs, choose **This Phone / Browser** to select an MP3Quran timed reciter and sync playback to the current ayah, or choose the device speaker. The Azan request plays the device's notification tone; recorded Adhan audio is not included. The ESP32 screen shows a microphone animation while listening, a recognition animation while Gemini is working, and the selected Surah name during playback. Never place the API key in dashboard JavaScript or commit it to the repository; rotate keys pasted into chat or logs.

### Open the dashboard later through your router or hotspot

After the ESP32 joins your router or phone hotspot, it receives a **local IP address from that network using DHCP**. This address is not fixed and may change. Find the **Home network IP** in the dashboard, or check the router/hotspot's connected-device list or the ESP32 serial log. Then connect your phone/computer to the **same router or hotspot** and open **http://<ESP32-home-IP>** in its browser (for example, `http://192.168.1.42`).

If you use a mobile hotspot, its owner phone provides the Wi-Fi connection to the ESP32. Some phones do not allow the hotspot-owner phone to reach connected clients; if the dashboard will not open on that phone, connect a second phone or computer to the hotspot and use the ESP32's displayed Home network IP there.

The header's **Modules** menu links to the overview, setup guide, prayer schedule, prohibited-time windows, clock/calendar screen controls, Pomodoro history, weather, Dhikr, Quran audio, Wi-Fi setup, and device settings. The setup guide gives the basic Wi-Fi, location, and touch steps. The dashboard exposes device status, today's prayer times, all three calculated prohibited-time windows, Hijri date, weather/AQI and OpenWeather pollutant readings, screen selection, Pomodoro controls, Dhikr count controls, Quran voice/player controls, and the active azan stop control. Wi-Fi scanning shows a progress dialog and returns selectable nearby networks. Router credentials are kept in RAM during connection testing and written to LittleFS only after association, a valid DHCP lease, and read-back verification; the password field is cleared after a successful connection and the dashboard does not store it in browser storage. Settings are range-validated, saved to LittleFS, and reloaded by the dashboard after saving; storage failures are reported without applying unsaved values. Service keys are not displayed or editable in the dashboard.

The Prayer-Bot setup network uses its default hotspot name, password, and fixed address `192.168.4.1`; these are not configurable in the dashboard. Its captive portal opens the setup page automatically on most phones and computers; if it does not, use the device's **Sign in to network** notification or the fallback address above. Router credentials are saved only after association, a valid DHCP address and gateway, and storage read-back verification; then the device restarts. Wi-Fi scanning can populate the router SSID but cannot reveal its password; enter the password manually. Browser voice input uses the browser's speech-recognition support and requires microphone permission. Revoke any API key pasted into chat or public logs and provision replacements privately; API keys are not hardcoded in tracked source.

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
- **Online APIs** – Aladhan provides prayer times; OpenWeatherMap supplies weather and air-quality data; Gemini identifies dashboard Surah and Azan requests; MP3Quran v3 supplies reciter and ayah-timing metadata. Keys stay in private device configuration and are not exposed by the dashboard API.

Feel free to fork, modify, and extend the bot (e.g., add voice commands or additional sensors). Happy building!

## 🛠️ Development Setup (VS Code + PlatformIO)

1. **Install VS Code** – download from https://code.visualstudio.com/ and run the installer.
2. **Add the PlatformIO extension** – open VS Code, go to Extensions (Ctrl+Shift+X), search for **PlatformIO IDE** and install it.
3. **Open the project** – choose **File → Open Folder…** and select the `e:\Prayer-Bot` directory.
4. **Build the firmware** – press the **PlatformIO: Build** button (or run `pio run` in the integrated terminal).
5. **Upload to the ESP32‑S3** – press **PlatformIO: Upload** (or `pio run --target upload`). Ensure the board is connected via USB.
6. **Serial monitor** – open **PlatformIO: Serial Monitor** (or `pio device monitor -b 115200`) to view boot logs and interact with the device.

> PlatformIO is the canonical, tested build system and installs pinned dependencies from `platformio.ini`. The root `Prayer-Bot.ino` is an Arduino IDE entry point for the modular sources under `src/`, not a standalone single-file build. The default configuration targets N16R8 (16 MB flash, 8 MB PSRAM); verify these match your board before flashing, especially if using an N8 variant. Create the ignored `src/secrets.h` before building. Keep the environment and dependency versions pinned unless firmware and hardware are retested.

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
