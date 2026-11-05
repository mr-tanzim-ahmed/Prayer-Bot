# Prayer-Bot Setup Guide

[📘 **Project README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md)

This guide covers installing the tools, building and uploading the firmware from VS Code, configuring Wi-Fi, and opening the device dashboard.

## Contents

- [Requirements](#requirements)
- [Install VS Code and PlatformIO](#install-vs-code-and-platformio)
- [Get the project and configure secrets](#get-the-project-and-configure-secrets)
- [Build, upload, and monitor](#build-upload-and-monitor)
- [Optional: configure dashboard HTTPS](#optional-configure-dashboard-https)
- [Connect Prayer-Bot to Wi-Fi](#connect-prayer-bot-to-wi-fi)
- [Set location and use the dashboard](#set-location-and-use-the-dashboard)
- [Troubleshooting](#troubleshooting)

## Requirements

### Software

- A computer running Windows, macOS, or Linux.
- [Visual Studio Code](https://code.visualstudio.com/).
- The **PlatformIO IDE** VS Code extension.
- An internet connection for the first build and for online device features.
- [OpenSSL](https://www.openssl.org/) only if you want to create a self-signed HTTPS certificate.

### Board and hardware

- ESP32 Dev Board (CH340) with a USB data cable (some cables provide power only).
- Display, touch sensor, motion sensor, amplifier, and speaker listed in the [hardware guide](HARDWARE_CONNECTIONS.md).
- An INMP441 microphone only for voice selection on the device; dashboard voice input uses the phone or computer microphone.
- An HMC5883L or QMC5883L compass is required for the Qibla screen.
- This build's power setup also uses an LM2596S adjustable step-down converter and a TP4056 single-cell battery charging module.

Review the [hardware connections](HARDWARE_CONNECTIONS.md) and [Prayer-Bot circuit diagram](CIRCUIT.md) before powering the board. In particular, never connect either MAX98357A speaker output to ground. The LM2596S cannot boost the TP4056's single-cell battery voltage to 5 V; check the power guidance and your exact module versions before wiring or charging.

## Install VS Code and PlatformIO

1. Install [Visual Studio Code](https://code.visualstudio.com/).
2. In VS Code, open **Extensions** from the left sidebar, or press **Ctrl+Shift+X**.
3. Search for **PlatformIO IDE** by PlatformIO and select **Install**.
4. Reload or restart VS Code if prompted.

PlatformIO reads the project configuration in [`platformio.ini`](platformio.ini) and installs the ESP32 toolchain and pinned libraries automatically during the first build. You do not need to install the Arduino IDE or the libraries separately.

## Get the project and configure secrets

Download or clone the repository. In VS Code, select **File → Open Folder…** and open the `Prayer-Bot` project root—the folder containing `platformio.ini`. Wait for PlatformIO to initialize the project.

Open the VS Code integrated terminal with **Terminal → New Terminal** and create the local secrets file from the example if it does not already exist:

```powershell
if (-not (Test-Path configuration\secrets.h)) { Copy-Item configuration\secrets.h.example configuration\secrets.h }
```

Open `configuration\secrets.h` in the Explorer. Add API keys if you want weather/air-quality data (OpenWeatherMap) or Quran request recognition (Gemini). Prayer times do not require your own API key. Wi-Fi credentials are optional; you can choose a network from the setup dashboard instead. Keep the file private: Git ignores `configuration\secrets.h`, so do not force-add or share it. If the file already exists, preserve your current keys; do not overwrite it.

## Build, upload, and monitor

1. Connect the ESP32 to your computer with a USB data cable.
2. In the VS Code status bar, select **✓ (Build)**. The first build may take several minutes while PlatformIO downloads the toolchain and dependencies.
3. Select **→ (Upload)** and choose the ESP32 serial port if prompted.
4. Select **plug (Serial Monitor)** to see startup logs at **115200 baud**. Close Serial Monitor before uploading again.

If the PlatformIO actions are not visible, open the PlatformIO icon in the Activity Bar or press **Ctrl+Shift+P** and run **PlatformIO: Build**, **PlatformIO: Upload**, or **PlatformIO: Monitor**.

If upload cannot connect, close Serial Monitor, check the USB data cable and serial driver, select the correct port, and retry. Some boards require holding **BOOT** while the upload begins.

The integrated terminal can also run the PlatformIO CLI:

```powershell
pio run
pio run --target upload
pio device monitor -b 115200
```

When multiple boards are connected, select the correct port in PlatformIO or specify it explicitly, for example:

```powershell
pio run --target upload --upload-port COM5
```

The serial log prints the setup dashboard address (`10.77.0.1`) and, after router setup, the home Wi-Fi address. HTTP is available on both interfaces; HTTPS is available on both when the HTTPS certificate files are installed and the listener starts.

> **Flash layout:** The 4 MB ESP32 target uses a single 2 MB firmware slot and about 1.875 MB for LittleFS. This provides room for the current firmware and persistent files but does not support A/B OTA updates. Changing partition layouts changes the LittleFS offset; back up settings and Wi-Fi credentials, then re-upload filesystem contents.

## Touch feedback sound

Touch feedback uses an MP3 embedded in program flash and plays through the MAX98357A I²S amplifier. The sound is included in the firmware, so no filesystem upload is needed for touch feedback.

## Optional: configure dashboard HTTPS

The dashboard serves both HTTP on port 80 and HTTPS on port 443. HTTP remains available if HTTPS has not been configured or cannot start. HTTPS is needed for browser features that require a secure context, including device geolocation and direct microphone recording.

The HTTPS server reads `data/https_certificate.pem` and `data/https_private_key.pem` from LittleFS. If you do not already have a certificate pair, create a local self-signed pair with OpenSSL. Replace the example router IP with the device's current address:

```powershell
openssl req -x509 -newkey rsa:2048 -noenc -keyout data\https_private_key.pem -out data\https_certificate.pem -days 3650 -subj "/CN=Prayer-Bot" -addext "subjectAltName=DNS:Prayer-Bot,DNS:prayer-bot.local,IP:10.77.0.1,IP:192.168.1.104"
```

Upload the firmware and filesystem so the device runs the HTTPS server and has the certificate pair in LittleFS:

```powershell
pio run --target upload
pio run --target uploadfs
```

Open `http://10.77.0.1/` or `https://10.77.0.1/` while connected to the setup Wi-Fi. On your router network, use the device's current IP with either `http://` or `https://`. Repeat the filesystem upload whenever you replace the certificate pair. **Never commit or share the private key.** Browsers may show a warning for a self-signed certificate; the certificate must match the dashboard address you use. HTTPS encrypts traffic but does not add dashboard authentication, so use the device only on a trusted local network.

The trusted root certificates in [`include/network/https_client.h`](include/network/https_client.h) are used for outbound HTTPS API requests made by the ESP32. They are separate from the dashboard's certificate files.

## Connect Prayer-Bot to Wi-Fi

1. Power on the ESP32. The device starts the visible **Prayer-Bot** setup access point on 2.4 GHz, even if it also tries previously saved router credentials. Select **Prayer-Bot** in your phone or computer's Wi-Fi list and connect with password **`12345678`**. If prompted, choose to stay connected despite there being no internet.
2. Tap the captive-portal notification. The ESP32 serves the dashboard for common phone/OS captive-network checks; whether a notification opens automatically is controlled by the phone. If it does not appear, stay connected to **Prayer-Bot** and open **http://10.77.0.1/** in a browser (use HTTP, not HTTPS).
3. Scan for and select any nearby compatible Wi-Fi network, or enter its SSID manually. Enter its password, then select **Connect to Wi-Fi**. This works with routers and device hotspots; the ESP32 hardware supports **2.4 GHz Wi-Fi only**, and WPA2-Personal (AES) is recommended. WPA3-only or enterprise networks may not be compatible.
   - **Android hotspot:** Keep the hotspot on and discoverable; enable its 2.4 GHz band and WPA2-Personal security when those options are available.
   - **iPhone Personal Hotspot:** Enable **Settings → Personal Hotspot → Maximize Compatibility** if available, then use the iPhone's displayed Wi-Fi password.
   - If the device providing the hotspot cannot stay connected to Prayer-Bot's setup Wi-Fi at the same time, use a second phone or computer to open the setup dashboard while that device shares its connection.
4. The device verifies the Wi-Fi connection and saves the credentials. This can take up to 35 seconds; it restarts after successful setup. If the attempt fails, the setup hotspot stays available so you can check the SSID/password or try another compatible router or phone hotspot.
5. Find the assigned address in Serial Monitor or the connected network's client list. Connect your browser device to the same Wi-Fi network and open that address.

The setup hotspot remains active while the device connects to Wi-Fi. Phones may ask whether to remain connected to this no-internet network. Use the password 12345678 to join the hotspot, and enter the chosen network details in the dashboard.

The setup hotspot uses `http://10.77.0.1` (or `https://10.77.0.1` when HTTPS is configured). HTTP and HTTPS are both served by the device; HTTPS may show a self-signed-certificate warning. The address on the selected Wi-Fi network is assigned by that network's DHCP and can change. Some phones cannot access a device connected to their own hotspot; if you cannot reach the dashboard, try another device on that network.

## Set location and use the dashboard

The dashboard offers device status, prayer times, prohibited-time windows, Hijri date, weather/AQI, screen selection, Pomodoro controls/history, Dhikr, Quran audio, Wi-Fi setup, and device preferences.

Dhaka is the default location. In **Device Preferences & Coordinates**, choose a preset or enter the city/area, coordinates, timezone, and UTC offset manually. The optional **Use device location** button requests permission from the browser and fills in coordinates and timezone. It requires HTTPS and the browser/device location service. Review the city/area label before saving; the LAN address alone cannot identify your geographic area, and the dashboard does not reverse-geocode coordinates. Saved coordinates are used for prayer times and, when configured, weather.

Browser location and direct microphone recording need a secure HTTPS page. Accept the self-signed certificate warning if using a locally generated certificate; if you stay on HTTP, enter location manually and use text-based requests or supported browser speech recognition instead of direct recording.

Typed Gemini requests work over HTTP or HTTPS. For direct audio recording, open HTTPS, allow microphone access, record for up to six seconds, then stop and send. The firmware forwards the recording to Gemini and does not store it. Phone browsers may require tapping the visible audio controls to start Quran playback.

## API keys

The firmware uses:

| Service | Purpose | Key required? |
|---|---|---|
| [Aladhan](https://aladhan.com/prayer-times-api) | Prayer timetable for the configured location and calculation method | No |
| [OpenWeatherMap](https://openweathermap.org/api) | Weather and air-pollution readings | Yes |
| [Google Gemini](https://aistudio.google.com/) | Recognizes typed or spoken Surah and Azan-notification requests | Yes |
| [MP3Quran v3](https://www.mp3quran.net/eng/api) | Reciter metadata and Quran audio sources | No |

Add the optional keys to `configuration\secrets.h`, then rebuild and upload the firmware. The keys are compiled into the device and are not shown or editable in the dashboard. Never commit, screenshot, or share API keys. If a key is exposed, revoke it with its provider and create a replacement.

## Troubleshooting

### The setup page did not open

If **Prayer-Bot** does not appear in the Wi-Fi list, power-cycle the ESP32 and check its startup messages in the Serial Monitor at **115200 baud**. A successful setup AP prints `[WiFi] AP started: Prayer-Bot` and `[WiFi] AP IP: 10.77.0.1`; `[WiFi] Failed to start visible setup access point` means the AP did not start. If those messages are absent, upload the firmware to the ESP32 and restart it. Keep the board powered and close to the phone while scanning; the setup AP is a 2.4 GHz network.

Once connected, stay on the **Prayer-Bot** Wi-Fi even if the phone reports no internet, then open **http://10.77.0.1/** manually.

### The dashboard is unreachable after connecting to the router

Check Serial Monitor or your router's client list for the current IP. Confirm the browser device is on the same network. The router-assigned IP may change after reconnecting; create a DHCP reservation in your router to keep it stable. If the device IP is 192.168.1.104, use `http://192.168.1.104/` or `https://192.168.1.104/` (HTTPS requires a matching certificate SAN and may show a trust warning). While connected directly to Prayer-Bot Wi-Fi, use `http://10.77.0.1/` or `https://10.77.0.1/`.

### Prayer times or Quran audio are unavailable offline

Prayer timings and their Hijri date are kept in a rolling three-day device backup, so prayer/Sehri times and derived prohibited windows remain available offline during that period. After an internet time sync, the clock and Gregorian calendar continue running offline while the board stays powered. After full power loss, Wi-Fi is needed again to set the clock because the documented circuit has no battery-backed RTC. When Wi-Fi reconnects, the device automatically resynchronizes its clock and refreshes prayer and weather data. An initial network connection is needed to fetch location-specific prayer timings. Surah recognition, Quran streaming, and weather updates also need internet; Dhikr and Pomodoro work locally.

### I use a different ESP32 board

The included PlatformIO configuration targets an ESP32 Dev Board (CH340) using the `esp32dev` profile. Verify flash size, pin availability, and voltage compatibility for your specific board before using it.

For physical connections and sensor troubleshooting, see the [hardware guide](HARDWARE_CONNECTIONS.md).
