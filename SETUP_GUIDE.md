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

- ESP32-WROOM-32 DevKit with a USB data cable (some cables provide power only).
- Display, touch sensor, motion sensor, amplifier, and speaker listed in the [hardware guide](HARDWARE_CONNECTIONS.md).
- An INMP441 microphone only for voice selection on the device; dashboard voice input uses the phone or computer microphone.
- An HMC5883L or QMC5883L compass is required for the Qibla screen.

Review the [hardware connections](HARDWARE_CONNECTIONS.md) before powering the board. In particular, never connect either MAX98357A speaker output to ground.

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
if (-not (Test-Path src\secrets.h)) { Copy-Item src\secrets.h.example src\secrets.h }
```

Open `src/secrets.h` in the Explorer. Add API keys if you want weather/air-quality data (OpenWeatherMap) or Quran request recognition (Gemini). Prayer times do not require your own API key. Keep the file private: Git ignores `secrets.h`, so do not force-add or share it. If the file already exists, preserve your current keys; do not overwrite it.

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

> **Flash layout:** The 4 MB ESP32-WROOM target uses a single 2 MB firmware slot and about 1.875 MB for LittleFS. This provides room for the current firmware and persistent files but does not support A/B OTA updates. Changing partition layouts changes the LittleFS offset; back up settings and Wi-Fi credentials, then re-upload filesystem contents.

## Touch feedback sound

Touch feedback is generated directly over I2S as a short, low-gain tone. It does not need an MP3 file, audio conversion, or a LittleFS upload. This avoids additional MP3-decoder memory use and prevents a filesystem upload from overwriting saved settings or Wi-Fi credentials just to install a beep.

## Optional: configure dashboard HTTPS

HTTPS is optional. The dashboard can use HTTP if HTTPS has not been configured or cannot start. HTTPS is needed for browser features that require a secure context, including device geolocation and direct microphone recording.

The HTTPS server reads `data/https_certificate.pem` and `data/https_private_key.pem` from LittleFS. If you do not already have a certificate pair, create a local self-signed pair with OpenSSL. Replace the example router IP with the device's current address:

```powershell
openssl req -x509 -newkey rsa:2048 -noenc -keyout data\https_private_key.pem -out data\https_certificate.pem -days 3650 -subj "/CN=Prayer-Bot" -addext "subjectAltName=DNS:Prayer-Bot,DNS:prayer-bot.local,IP:10.77.0.1,IP:192.168.1.104"
```

Upload the filesystem image, which includes the certificate pair:

```powershell
pio run --target uploadfs
```

Repeat the filesystem upload whenever you replace the certificate pair. **Never commit or share the private key.** Browsers may show a warning for a self-signed certificate; the certificate must match the dashboard address you use. HTTPS encrypts traffic but does not add dashboard authentication, so use the device only on a trusted local network.

The trusted root certificates in [`src/https_client.h`](src/https_client.h) are used for outbound HTTPS API requests made by the ESP32. They are separate from the dashboard's certificate files.

## Connect Prayer-Bot to Wi-Fi

1. Power on the ESP32. On your phone or computer, select **Prayer-Bot** in the Wi-Fi list and connect with password **`12345678`**. If prompted, choose to stay connected despite there being no internet.
2. Tap the captive-portal notification. The ESP32 serves the dashboard for common phone/OS captive-network checks; whether a notification opens automatically is controlled by the phone. If it does not appear, stay connected to **Prayer-Bot** and open **http://10.77.0.1/** in a browser (use HTTP, not HTTPS).
3. Enter the SSID and password of a **2.4 GHz** router or phone hotspot, then select **Connect to Wi-Fi**.
4. The device verifies the router connection and saves the credentials. This can take up to 35 seconds; it restarts after successful setup.
5. Find the home Wi-Fi address in Serial Monitor or your router's connected-device list. Connect your browser device to the same network and open that address.

The setup hotspot remains active while the device connects to the router. Phones may ask whether to remain connected to this no-internet network. Use the password 12345678 to join the hotspot, and enter your home router details in the dashboard.

The setup hotspot uses `http://10.77.0.1` (or `https://10.77.0.1` when HTTPS is configured). HTTP and HTTPS are both served by the device; HTTPS may show a self-signed-certificate warning. The home Wi-Fi address is assigned by your router's DHCP and can change. To make it stable, create a DHCP reservation in the router; changing the ESP32's home-network address to a 10.x IP without matching router configuration would make it unreachable. Some phones cannot access devices connected to their own hotspot; if you cannot reach the dashboard, try another device on that network.

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

Add the optional keys to `src/secrets.h`, then rebuild and upload the firmware. The keys are compiled into the device and are not shown or editable in the dashboard. Never commit, screenshot, or share API keys. If a key is exposed, revoke it with its provider and create a replacement.

## Troubleshooting

### The setup page did not open

Stay connected to the **Prayer-Bot** Wi-Fi even if the phone reports no internet, then open **http://10.77.0.1/** manually.

### The dashboard is unreachable after connecting to the router

Check Serial Monitor or your router's client list for the current IP. Confirm the browser device is on the same network. The router-assigned IP may change after reconnecting; create a DHCP reservation in your router to keep it stable. If the device IP is 192.168.1.104, use `http://192.168.1.104/` or `https://192.168.1.104/` (HTTPS requires a matching certificate SAN and may show a trust warning). While connected directly to Prayer-Bot Wi-Fi, use `http://10.77.0.1/` or `https://10.77.0.1/`.

### Prayer times or Quran audio are unavailable offline

Previously fetched prayer data may be available from cache, but the timetable requires internet to fetch initially and refresh monthly. Weather, Surah recognition, and Quran streaming also need internet. Dhikr and Pomodoro work locally.

### I use a different ESP32 board

The included PlatformIO configuration targets an ESP32-WROOM-32 DevKit using the `esp32dev` profile. Verify flash size, pin availability, and voltage compatibility for your specific board before using it.

For physical connections and sensor troubleshooting, see the [hardware guide](HARDWARE_CONNECTIONS.md).
