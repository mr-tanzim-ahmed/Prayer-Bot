<div align="center">

# Prayer-Bot

### An open-source ESP32 prayer clock and Mumins Personal Assistant.

Prayer-Bot brings prayer times, a Hijri calendar, weather, Quran audio, Dhikr, and Pomodoro focus sessions together in a DIY project you can build, understand, and customize.

[![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32-orange?logo=platformio)](https://platformio.org/)
[![Version](https://img.shields.io/badge/version-1.0.1-blue)](https://github.com/mr-tanzim-ahmed/Prayer-Bot/releases/tag/v1.0.1)
[![License: MIT](https://img.shields.io/badge/License-MIT-2ea44f.svg)](LICENSE)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-00979D?logo=arduino&logoColor=white)](https://docs.platformio.org/en/latest/frameworks/arduino.html)

[🚀 Setup guide](SETUP_GUIDE.md) · [✨ Features](#-features) · [🔧 Hardware guide](HARDWARE_CONNECTIONS.md) · [📐 Circuit diagram](CIRCUIT.md)

<br>

<a href="CIRCUIT.md"><img src="circuit_image.svg" width="420" alt="Prayer-Bot circuit overview"></a>

<sub>ESP32 prayer clock with OLED display, touch controls, motion sensing, and audio · <a href="CIRCUIT.md">Explore the circuit and wiring</a></sub>

</div>

---

## ✨ Features

Prayer-Bot is a DIY **ESP32 prayer clock and Mumins Personal Assistant**. Use its OLED and touch sensor on your desk, or open its local web dashboard from a phone or computer on the same network.

| On the device | In the web dashboard | Built for developers |
|---|---|---|
| 🕌 Prayer focus and daily prayer schedule | 📱 Local dashboard and device controls | ⚙️ Modular C++ managers and screens |
| 🗓️ Clock, Gregorian/Hijri calendar, and events | 📍 Location, calculation, and device settings | 🧵 FreeRTOS tasks for display, network, and audio |
| 🧭 Qibla bearing and live compass direction | 📈 Seven-day Pomodoro history | 💾 LittleFS settings and cached data |
| ⏱️ Pomodoro focus timer and Dhikr counter | 🎧 Quran Surah requests by voice or text | 🔌 Aladhan, OpenWeatherMap, Gemini, and MP3Quran |
| 🔊 Azan notification tone and Quran audio | 🕹️ Screen, timer, Dhikr, and audio controls | 🔐 API keys stay on the device |

**Eight OLED screens:** Prayer Focus · All Prayers · Clock & Calendar · Qibla · Weather · Pomodoro · Dhikr · Quran.

The Qibla screen shows a large arrow that turns toward Qibla relative to the device, without a compass dial or extra readouts. A short message appears only when the compass, location, or calibration is not ready. Set another area in the dashboard or use its permission-based device-location option to update the bearing. The screen uses an HMC5883L or QMC5883L compass. On first use, keep the device flat and rotate it through a full turn (a figure-eight motion also helps) to calibrate hard-iron offsets; the offsets are saved in LittleFS. Long-press the touch sensor on the Qibla screen to recalibrate. The bearing uses zero magnetic declination and is approximate. Keep the compass away from speakers, motors, magnets, and metal; magnetic interference or an incorrectly oriented module can make the direction inaccurate.

Touch feedback is generated directly over I2S as a short tone and follows the configured device volume. This avoids MP3 decoding and LittleFS file dependencies for a small notification sound, which is more reliable on the low-memory ESP32 target. No audio asset or filesystem upload is required. Touch feedback is skipped during Quran playback and interrupted by the Azan alarm. Make sure the amplifier and speaker are wired safely before powering them.

The setup hotspot SSID is **`Prayer-Bot`** (password: **`12345678`**) at default IP **`http://10.77.0.1`**. Connecting from a smartphone or laptop automatically triggers a captive-portal prompt ("Sign in to network") that opens the dashboard immediately. The dashboard is also directly accessible in any browser at `http://10.77.0.1` (or `https://10.77.0.1` when certificates are installed). Once connected to home Wi-Fi, the router-side IP remains assigned by your router DHCP.

While **All Prayers** is selected, the OLED shows the five daily prayers in clear rows, with each prayer name on the left and its time on the right, for 3 seconds. It then shows Sahari end (Fajr) and Iftar (Maghrib) for 2 seconds. This 5-second cycle repeats until you advance to another screen.

The dashboard prayer schedule also shows Sahari end and Iftar times, using the day's Fajr and Maghrib times.

The OLED startup screen presents a centered “Assalamualaikum” greeting and its translation, sized to fit within the display frame.

> **Audio note:** The Azan alert is a repeating notification tone. Recorded Adhan audio is not included. The Quran assistant identifies Surah requests; it does not provide Quran translation, tafsir, or free-form Quran-text search.

## Get started

For the complete VS Code and PlatformIO walkthrough (extension installation, secrets setup, firmware build/upload, Wi-Fi provisioning, dashboard access, and troubleshooting), follow the **[Prayer-Bot Setup Guide](SETUP_GUIDE.md)**.

Before powering the board, review the [hardware connections](HARDWARE_CONNECTIONS.md) and [circuit diagram](CIRCUIT.md). PlatformIO installs the dependencies listed in [`platformio.ini`](platformio.ini) during the first build.

## 🧭 Where to start in the code

The firmware uses focused C++ managers for feature logic and separate modules for the OLED screens. If you are new to the codebase, start with `src/main.cpp` to see how the device starts up and coordinates its FreeRTOS tasks. For a feature change, begin in the matching manager or screen module:

| If you want to change… | Start here |
|---|---|
| Startup, task scheduling, or module coordination | [`src/main.cpp`](src/main.cpp) |
| Pins, defaults, timing, or API endpoints | [`src/config.h`](src/config.h) |
| Prayer schedules and prohibited-time windows | [`src/prayer_manager.h`](src/prayer_manager.h), [`src/prayer_manager.cpp`](src/prayer_manager.cpp) |
| Dashboard UI or its JSON API | [`src/dashboard_html.h`](src/dashboard_html.h), [`src/web_server_manager.cpp`](src/web_server_manager.cpp) |
| HTTPS listener and device-local certificate | [`src/https_server_manager.cpp`](src/https_server_manager.cpp) |
| Voice recognition and Quran playback | [`src/voice_manager.cpp`](src/voice_manager.cpp), [`src/screens/`](src/screens/) |
| Pomodoro state and weekly statistics | [`src/pomodoro_manager.cpp`](src/pomodoro_manager.cpp), [`src/pomodoro_stats.cpp`](src/pomodoro_stats.cpp) |
| Settings, LittleFS, or caches | [`src/storage_manager.cpp`](src/storage_manager.cpp), [`src/cache_manager.cpp`](src/cache_manager.cpp) |
| OLED presentation | [`src/screens/`](src/screens/), [`src/screen_router.cpp`](src/screen_router.cpp) |

Shared state and screen IDs live in [`src/types.h`](src/types.h). Most managers have a matching `.h` interface and `.cpp` implementation. Keep feature behavior close to the module that owns it; use `main.cpp` to initialize services and connect them. If responsibilities or filenames change, update this map so the next contributor can find their way.

## 🧰 Hardware components and wiring

These are the main parts used by the ESP32 prayer clock. The INMP441 microphone is optional; it is needed only for voice selection on the device itself. Browser voice input uses the phone or computer microphone.

| Part | Role | Required? |
|---|---|---|
| ESP32-WROOM-32 DevKit (CH340 USB-C) | Main controller, Wi-Fi, and storage | Yes |
| 1.3-inch I²C SH1106 OLED | Device screens | Yes |
| TTP223B capacitive touch sensor | Screen navigation and quick actions | Yes |
| MPU6050 / GY-521 | Shake controls | Yes |
| HMC5883L or QMC5883L compass (GY-271 / GY-273 boards) | Qibla screen compass direction and bearing | Required for Qibla; optional for other features |
| MAX98357A I²S amplifier + speaker | Alerts and Quran audio | Yes for device audio |
| INMP441 I²S microphone | On-device Surah voice selection | Optional |

<details>
<summary><strong>GPIO pin map</strong></summary>

| Signal | ESP32 GPIO |
|---|---:|
| OLED + MPU6050 + compass I²C SDA | 21 |
| OLED + MPU6050 + compass I²C SCL | 22 |
| Capacitive touch output | 4 |
| MAX98357A BCLK / LRC (WS) / DIN | 26 / 25 / 27 |
| INMP441 SCK / WS / SD | 16 / 17 / 18 |

The OLED, MPU6050, and optional compass share an I²C bus; the MAX98357A amplifier and INMP441 microphone use separate I²S connections. Use 3.3 V-compatible logic and connect the modules to a common ground. **Do not connect either speaker output terminal to ground.** If the amplifier or wiring sparks, disconnect power to the amplifier and speaker and do not reconnect them until the cause has been found; leaving SD disconnected does not make a short safe. Check the voltage requirements printed on your specific breakout boards before applying power.

The compass driver supports HMC5883L sensors at I²C address `0x1E` and QMC5883L sensors at `0x0D`, auto-detecting which chip is fitted. The sensor is expected to be mounted flat with its X axis toward the display's right edge and Y axis toward the top edge. The firmware checks the HMC5883L identity or verifies QMC5883L configuration, rejects overflow or stale readings, retries sensor communication, and requests a full-turn calibration before showing the arrow. Calibration corrects hard-iron offsets, but does not correct soft-iron distortion, tilt, or local magnetic declination. The displayed true-north bearing is therefore approximate, not guaranteed to be within a particular percentage or degree error. The firmware also expects the OLED at `0x3C` and MPU6050 at `0x68`; unlike the supplied diagnostic, it does not try alternate addresses.

See [HARDWARE_CONNECTIONS.md](HARDWARE_CONNECTIONS.md) for full wiring notes and [CIRCUIT.md](CIRCUIT.md) for the illustrated connection reference. Pin assignments are defined in [`src/config.h`](src/config.h).

</details>

## Services and API keys

Prayer schedules use Aladhan and do not require a personal API key. Weather and air-quality features need an optional OpenWeatherMap key; Gemini recognition of Quran requests needs an optional Gemini key. Store optional keys in the local, Git-ignored `src/secrets.h` file. See the [Setup Guide](SETUP_GUIDE.md#api-keys) for safe key setup and service details.

Prayer schedules are fetched as a monthly calendar and cached on-device. A network connection is needed for the first fetch and later month refreshes; this is not a preloaded offline timetable. Weather and prayer data refresh when online. Default location: Dhaka, Bangladesh (23.8103, 90.4125), `Asia/Dhaka` (UTC+6).

## 🎛️ Touch, shake, and dashboard controls

Use the touch sensor for everyday controls, or use the dashboard when your phone or computer is on the same network as the device.

| What you do | What happens |
|---|---|
| Tap touch sensor | Advance OLED screen; on Dhikr, increment the count |
| Shake device | Dismiss an active Azan alert; control playback/timer on the relevant screen |
| Hold touch for 3 seconds on Pomodoro | Start or pause/resume the timer |
| Tap during a Pomodoro break-ready prompt | Start the break |
| Dashboard controls | Select screens, control timer and Dhikr, and manage Quran playback |

<details>
<summary><strong>Pomodoro behavior and settings</strong></summary>

- Defaults: **25-minute focus**, **5-minute short break**, **15-minute long break** after **4** focus cycles.
- The touch-started break delay is configurable from **2 to 5 minutes**.
- At focus completion, the device beeps and waits for a touch or dashboard action before starting the break. Focus resumes automatically after the break.
- A shake starts an idle timer or pauses/resumes an active timer while the Pomodoro screen is open. A shake does not start a break that is waiting for touch.
- The dashboard’s seven-day history tracks sessions started, completed focus sessions, and completed focus minutes. Partial or interrupted focus sessions do not count as completed.

</details>

<details>
<summary><strong>Dhikr phrases</strong></summary>

| Phrase | Meaning |
|---|---|
| Subhan Allah | Glory be to Allah |
| Alhamdulillah | Praise be to Allah |
| La ilaha illallah | There is no deity except Allah |
| Allahu Akbar | Allah is the Greatest |
| Astaghfirullah | I seek Allah’s forgiveness |

The OLED cycles through English transliterations and meanings; no Arabic font or image assets are required.

</details>

## 🧱 Firmware architecture and development notes

- **Board:** PlatformIO uses the `esp32dev` profile for a typical ESP32-WROOM-32 DevKit. Board variants may differ in flash size and pin availability.
- **Flash layout:** The 4 MB target uses `no_ota.csv`: one 2 MB firmware slot and about 1.875 MB for LittleFS. This preserves the current feature set but does not support A/B OTA updates. Changing from the default partition layout changes the LittleFS offset; back up settings and Wi-Fi credentials, then re-upload the LittleFS image after changing layouts.
- **Memory:** The ESP32-WROOM-32 target has no PSRAM by default. Microphone I²S DMA buffers are sized for this target, and dashboard voice uploads use PSRAM when available with an internal-RAM fallback; very large uploads can still be rejected if the largest available memory block is too small.
- **Framework and dependencies:** Arduino framework with dependency versions pinned in [`platformio.ini`](platformio.ini).
- **Concurrency:** FreeRTOS tasks coordinate display rendering, networking, scheduling, audio, and voice features.
- **Persistent data:** Settings, cached responses, and bounded Pomodoro history are stored in LittleFS. Prayer and weather cache files have expiry policies; interrupted atomic writes leave `.tmp` files that are cleaned at startup. Serial startup logs report filesystem usage. Back up device settings before replacing or erasing the filesystem.
- **JSON and web requests:** ArduinoJson handles service responses. Dashboard request bodies are size-limited and validated before settings are applied.

## Setup help

For VS Code/PlatformIO installation, firmware upload, Wi-Fi, HTTPS, location configuration, and troubleshooting, see the **[Setup Guide](SETUP_GUIDE.md)**. For wiring and sensor checks, see [HARDWARE_CONNECTIONS.md](HARDWARE_CONNECTIONS.md).

## 🤝 Contributing

Have an idea, found a bug, or want to improve the ESP32 prayer clock? Issues and pull requests are welcome. Before submitting a change, keep feature logic in the responsible manager or screen, follow the existing pin and configuration conventions, and update the README or wiring documentation when behavior changes.

---

<div align="center">

**Ready to build your ESP32 prayer clock?** Start with the [VS Code and PlatformIO setup guide](SETUP_GUIDE.md), then follow the [hardware and wiring guide](HARDWARE_CONNECTIONS.md).

Made to be built, understood, and extended. · [MIT License](LICENSE)

</div>
