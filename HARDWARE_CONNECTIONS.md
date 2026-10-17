# Hardware Connections for Prayer‑Bot

[📄 **README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md) | [📜 **License**](LICENSE)

This document describes the GPIO wiring between the **ESP32‑S3 (DevKitC‑1)** and all peripheral modules used in the project.

| Component | Component Pin | ESP32‑S3 Connection | Notes |
|-----------|--------------|---------------------|-------|
| **1.3″ OLED Display (I2C)** | SDA | **GPIO 11** | I2C data line. Use a 4.7 kΩ pull‑up to 3.3 V (already on many modules). |
| | SCL | **GPIO 12** | I2C clock line. 4.7 kΩ pull‑up to 3.3 V. |
| | VCC / GND | **3.3 V / GND** | Power the OLED. |
| **MPU6050 / GY‑521 (I2C)** | SDA | **GPIO 11** | Shared I2C bus with the OLED display. |
| | SCL | **GPIO 12** | Shared I2C bus with the OLED display. |
| | VCC / GND | **3.3 V / GND** | Power the IMU. |
| **TTP223B Capacitive Touch** | SIG / OUT | **GPIO 4** | Default active-HIGH output. A touch advances to the next screen, counts dhikr, starts Pomodoro break, and dismisses Azan. |
| | VCC / GND | **3.3 V / GND** | Power the touch sensor. |
| **MAX98357A I2S Amplifier** | BCLK | **GPIO 5** | I2S bit clock. |
| | LRC / WS | **GPIO 6** | I2S word select (left/right clock). |
| | DIN | **GPIO 7** | I2S data from ESP32 to amplifier. |
| | VIN / GND | **3.3 V / GND** | Supply the amp. |
| | GAIN | Leave unconnected | Uses default gain. |
| | SD | Do not connect to GND | Leave floating or pull high to enable. |
| **Speaker** | SPK+ / SPK− | Amplifier outputs | Connect directly to MAX98357A speaker terminals. |
| **INMP441 I2S Microphone** | SCK | **GPIO 16** | I2S bit clock for mic input. |
| | WS | **GPIO 17** | I2S word select for mic input. |
| | SD (data output) | **GPIO 18** | Audio data from mic to ESP32. |
| | L/R | **GND** | Left‑channel selection. |
| | VDD / GND | **3.3 V / GND** | Power the microphone. |
| **Power Supply** | USB‑C / VIN | **5 V** | Provide 5 V via the board's USB‑C connector or a regulated supply. |
| | 3.3 V regulator | **3.3 V** | Internally generated for logic. |
| | GND | Ground | All components share this ground. |

### Wiring Tips
- Keep **I2C** lines short (under 10 cm) to preserve signal integrity.
- Use **common ground** for all peripherals; a single GND rail on a small breadboard works well.
- The I2S pins (BCLK, LRC, DIN) must be routed together to minimise skew – keep the audio wires as short as possible.
- The INMP441 uses a **separate I2S port** (I2S_NUM_0) from the amplifier (I2S_NUM_1) – they do **not** share pins.
- The firmware debounces the touch sensor in software.
- The touch input is read as a digital signal; use the TTP223B's default active-HIGH output mode.
- On ESP32-S3 boards with the onboard addressable RGB LED on GPIO 48, the firmware flashes it white twice after a successful Wi-Fi connection.
- Shake detection uses the **MPU6050 accelerometer** on the shared I2C bus – no separate vibration sensor is needed.

### Visual Summary (ASCII schematic)
```
 ESP32‑S3                    OLED (I2C)           MAX98357A (I2S)
+----------+              +-----------+        +----------------+
|          |              |           |        |                |
|  GPIO11  +-- I2C SDA ---+ SDA       |        | BCLK  <-- GPIO5
|  GPIO12  +-- I2C SCL ---+ SCL       |        | LRC   <-- GPIO6
|          |              | VCC 3.3V  |        | DIN   <-- GPIO7
|          |              | GND       |        | VIN 3.3V       |
|          |              +-----------+        | GND            |
|          |                                   +-------+--------+
|          |      MPU6050 (I2C shared)                 |
|          |     +----------+                    Speaker +/−
|  GPIO11  +-----+ SDA      |
|  GPIO12  +-----+ SCL      |         INMP441 (I2S Mic)
|          |     | VCC 3.3V |        +----------------+
|          |     | GND      |        | SCK   <-- GPIO16
|          |     +----------+        | WS    <-- GPIO17
|          |                         | SD    --> GPIO18
|   GPIO4  +-- TTP223B OUT          | L/R   --> GND
|          |                         | VDD 3.3V       |
+----------+                         +----------------+
```

### Pin Summary
- **I2C Bus**: GPIO 11 (SDA) / GPIO 12 (SCL) – shared by OLED and MPU6050.
- **I2S Amplifier**: GPIO 5 (BCLK) / GPIO 6 (LRC) / GPIO 7 (DIN).
- **I2S Microphone**: GPIO 16 (SCK) / GPIO 17 (WS) / GPIO 18 (SD).
- **Touch Sensor**: GPIO 4 (single TTP223B).
- **Power**: 5 V via USB‑C, 3.3 V regulator for logic.

Follow this map when assembling your hardware on a breadboard or PCB, and the firmware will detect each peripheral automatically.
