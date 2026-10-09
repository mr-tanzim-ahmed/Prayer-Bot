# Prayer-Bot: Smart Islamic Personal Assistant (ESP32‑S3)

Welcome to **Prayer‑Bot**, a DIY, offline‑first digital companion that helps you manage daily prayers, track Hijri dates, stay focused with a Pomodoro timer, and monitor local weather—all from a single ESP32‑S3 desk clock.

---

## 🌟 Key Features

- **🕌 Automated Prayer Times & Azan** – Accurate monthly schedules from the free Aladhan API with optional I2S audio playback.
- **🌙 Hijri Calendar & Event Tracker** – Current Islamic date, Ramadan countdown, major holidays, and custom offset handling.
- **⏱️ Pomodoro Focus Timer** – 25 min work / 5 min break cycles, controllable via a physical button.
- **🌤️ Live Weather & AQI** – Real‑time temperature, humidity, and air‑quality data from Open‑Meteo.
- **📱 Web Dashboard (Captive Portal)** – Configure Wi‑Fi, city/division, calculation method, and timer settings from any smartphone.
- **📳 Shake‑to‑Snooze** – Vibration sensor lets you dismiss or snooze alerts with a shake.
- **🕋 Dhikr Screen** – 5‑second Dhikr phrases shown for a total of 100 s, auto‑returning to the main screen.
- **🔧 Robust Memory Management** – All JSON parsing is streamed directly into LittleFS; no dynamic allocations for new features.

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

## 🚀 Getting Started

1. **Flash the Firmware** – Open the project in VS Code with PlatformIO (`platformio.ini` is pre‑configured) and run `PlatformIO: Build` → `Upload`.
2. **First Boot & Wi‑Fi Setup** – On power‑up the device creates its own Wi‑Fi AP. Connect with your phone, open the captive‑portal dashboard, and enter your home Wi‑Fi credentials.
3. **Configure & Enjoy** – Set your city/division, prayer calculation method, audio volume, and Pomodoro cycles through the web UI. The device will fetch weather and prayer data, then operate fully offline.

---

## 📚 Under the Hood (Software & APIs)

- **FreeRTOS** – Separate tasks for networking, UI rendering, sensor reading, and audio playback.
- **Memory‑Efficient JSON Streaming** – Large monthly calendar payloads are parsed directly into flash storage.
- **Free APIs** – Aladhan (prayer times) and Open‑Meteo (weather/AQI) – no API keys required.

Feel free to fork, modify, and extend the bot (e.g., add voice commands or additional sensors). Happy building!
