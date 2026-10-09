# Prayer-Bot: Smart Islamic Personal Assistant (ESP32-S3)

Welcome to **Prayer-Bot**—a DIY, offline-first digital companion designed to help you seamlessly manage your daily prayers, track Hijri dates, maintain deep work focus, and stay updated on local weather. 

Powered by the robust **ESP32-S3** microcontroller, this smart desk clock acts as your personal Islamic assistant, offering a suite of productivity and spiritual tools packed into one sleek device.

---

## 🌟 Key Features

* **🕌 Automated Prayer Times & Azan:** Get pinpoint accurate monthly prayer schedules using the free Aladhan API. The clock provides visual alerts and can trigger I2S audio for beautiful Azan playback.
* **🌙 Hijri Calendar & Event Tracker:** Always know the current Islamic date. The bot tracks Hijri offsets and proactively reminds you of major events like Ramadan, Eid, and Jumuah.
* **⏱️ Pomodoro Focus Timer:** Boost your productivity with a built-in focus timer (25-minute work / 5-minute break cycles) controlled simply by a physical button.
* **🌤️ Live Local Weather & AQI:** Check real-time temperature, humidity, and Air Quality Index (AQI) powered by the keyless Open-Meteo API.
* **📱 Wireless Web Dashboard:** Configure your city, timezone, calculation methods, and timer durations straight from your phone via a local captive portal—no coding required!
* **📳 Shake to Snooze:** Built with a vibration sensor so you can physically shake the device to dismiss or snooze alerts.

---

## 🛠️ What You Need (Hardware Parts)

To build your own Prayer-Bot, you will need the following components:

* **Microcontroller:** ESP32-S3 Development Board (N16R8 variant with 16MB Flash and 8MB PSRAM is recommended for smooth audio handling).
* **Display:** 1.3" OLED Screen (I2C) for crisp, readable UI elements.
* **Audio Setup:** MAX98357A I2S Amplifier breakout + a small speaker (for Azan playback).
* **Sensors & Inputs:** 
  * SW-420 Vibration/Shake Sensor
  * Standard Push Button (for menu navigation and Pomodoro control)
  * INMP441 I2S Microphone (optional, for future voice commands)

---

## 🚀 How to Use & Setup

Getting your Prayer-Bot up and running is incredibly straightforward.

1. **Flash the Code:** 
   * **Using Arduino IDE:** Double-click the `Prayer-Bot.ino` file to open the project. The IDE will automatically recognize all the modular source files. Hit *Upload*.
   * **Using PlatformIO:** Open the repository in VS Code. All library dependencies are already configured in `platformio.ini`. Build and upload to your board.
2. **First Boot & Wi-Fi:** 
   * When powered on for the first time, Prayer-Bot will broadcast its own Wi-Fi Access Point. 
   * Connect to it using your smartphone and you will be greeted by the Web Dashboard.
3. **Configure & Enjoy:** 
   * Enter your home Wi-Fi credentials and type in your city name. 
   * Prayer-Bot will connect to your network, automatically fetch the latest weather and a full month's prayer schedule, and save it all to local memory for offline use!

---

## 🧠 Under the Hood (Software & APIs)

Prayer-Bot is heavily optimized to run smoothly 24/7 without crashing or exhausting the ESP32's memory.

* **FreeRTOS Driven:** The codebase is fully modular, running independent tasks for networking, the user interface, and sensor reading simultaneously.
* **Memory Efficient:** Uses advanced JSON stream filtering to download massive monthly calendar payloads, parsing only exactly what is needed directly into flash storage.
* **100% Free APIs:** No hidden costs or API keys required. We rely exclusively on the **Aladhan API** for Islamic timings and **Open-Meteo** for live weather data.
