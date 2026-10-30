# Hardware Connections for Prayer‑Bot

[📄 **README**](README.md) | [🔧 **Hardware Connections**](HARDWARE_CONNECTIONS.md) | [📐 **Circuit Diagram**](CIRCUIT.md) | [📜 **License**](LICENSE)

This document describes the GPIO wiring between a typical **ESP32-WROOM-32 DevKit (CH340 USB-C)** and the peripheral modules used in the project.

| Component | Component Pin | ESP32 Connection | Notes |
|-----------|--------------|---------------------|-------|
| **1.3″ OLED SH1106 (I2C)** | SDA | **GPIO 21** | I2C data line. Firmware expects address `0x3C`. Use a 4.7 kΩ pull-up to 3.3 V (already on many modules). |
| | SCL | **GPIO 22** | I2C clock line. 4.7 kΩ pull-up to 3.3 V. |
| | VCC / GND | **3.3 V / GND** | Power the OLED. |
| **MPU6050 / GY‑521 (I2C)** | SDA | **GPIO 21** | Shared I2C bus with the OLED and compass; firmware expects address `0x68`. |
| | SCL | **GPIO 22** | Shared I2C bus with the OLED and compass. |
| | VCC / GND | **3.3 V / GND** | Power the IMU. |
| **HMC5883L / QMC5883L compass (GY-271 / GY-273 boards)** | SDA | **GPIO 21** | Shared I2C bus with the OLED and MPU6050; HMC5883L address `0x1E`, QMC5883L address `0x0D`. Confirm the actual chip; board labels do not identify the sensor IC reliably. |
| | SCL | **GPIO 22** | Shared I2C bus with the OLED and MPU6050. |
| | VCC / GND | **3.3 V / GND** | Confirm the breakout's voltage rating and actual chip before connecting power. |
| | DRDY | Not connected | Optional data-ready signal is not used. |
| **TTP223B Capacitive Touch** | SIG / OUT | **GPIO 4** | Default active-HIGH output. A touch gives a short, quiet speaker beep and advances to the next screen, counts dhikr, starts Pomodoro break, or dismisses Azan. |
| | VCC / GND | **3.3 V / GND** | Power the touch sensor. |
| **MAX98357A I2S Amplifier** | BCLK | **GPIO 26** | I2S bit clock. |
| | LRC / WS | **GPIO 25** | I2S word select (left/right clock). |
| | DIN | **GPIO 27** | I2S data from ESP32 to amplifier. |
| | VIN / GND | **3.3 V / GND** | Use only a supply allowed by the exact amplifier breakout's rating. |
| | GAIN | Leave unconnected | Uses default gain. |
| | SD | Leave disconnected for now | Keep disconnected until the sparking issue is resolved. |
| **Speaker** | SPK+ / SPK− | Amplifier outputs | Connect directly to MAX98357A speaker terminals. |
| **INMP441 I2S Microphone** | SCK | **GPIO 16** | I2S bit clock for mic input. |
| | WS | **GPIO 17** | I2S word select for mic input. |
| | SD (data output) | **GPIO 18** | Audio data from mic to ESP32. |
| | L/R | **GND** | Left‑channel selection. |
| | VDD / GND | **3.3 V / GND** | Power the microphone. |
| **Power Supply** | USB‑C / VIN | **5 V** | Provide 5 V via the board's USB‑C connector or a regulated supply. |
| | 3V3 | **3.3 V** | Use only for modules rated for 3.3 V; keep the total load within the board regulator's rating. |
| | GND | Ground | All components share this ground. |

The wiring pin assignments in the supplied diagnostic match the firmware: I²C SDA/SCL are GPIO 21/22; touch is GPIO 4; INMP441 is GPIO 16/17/18; and MAX98357A is GPIO 26/25/27. This confirms pin-map consistency only, not that physical wiring or modules are working.

### Firmware and diagnostic coverage

| Device | Firmware expectation | Supplied diagnostic behavior |
|---|---|---|
| SH1106 OLED | GPIO 21/22, address `0x3C` | Scans `0x3C` or `0x3D` and initializes the display |
| MPU6050 | GPIO 21/22, address `0x68` | Tries `0x68` or `0x69` and reads sensor values |
| HMC5883L / QMC5883L compass | GPIO 21/22; firmware detects HMC5883L at `0x1E` or QMC5883L at `0x0D`; Qibla screen uses calibrated X/Y heading | Diagnostic detects HMC5883L at `0x1E` and reads raw values |
| TTP223B | GPIO 4, active-HIGH | Reads the digital touch output |
| INMP441 | I²S port 0: SCK/BCLK 16, WS 17, SD 18 | Installs I²S and reports input sample level |
| MAX98357A | I²S port 1: BCLK 26, LRC/WS 25, DIN 27 | Playback test is disabled by default, so the diagnostic does not verify amplifier or speaker operation |

Mount the HMC5883L or QMC5883L flat, with its X axis toward the display's right edge and its Y axis toward the top edge. On first use, open the Qibla screen and rotate the device flat through a full 360° turn; a figure-eight motion helps expose compass hard-iron offsets. The calibration is stored in `/compass_cal.json`; long-press the touch sensor on the Qibla screen to recalibrate. Keep the compass away from the speaker, amplifier, motors, magnets, and ferrous metal. The bearing assumes zero magnetic declination, so it is an approximation; tilt, soft-iron distortion, and nearby magnetic interference can cause additional error. Firmware auto-detects HMC5883L (`0x1E`) and QMC5883L (`0x0D`) modules; other compass chips are not supported.

The diagnostic's alternate OLED and MPU addresses are not supported by the current firmware configuration. Use OLED `0x3C` and MPU6050 `0x68` for the project firmware, or update and test the firmware before using modules configured at alternate addresses. The ESP32-WROOM-32 target has no PSRAM by default; dashboard audio uploads fall back to internal RAM and may be rejected if a sufficiently large contiguous block is unavailable. An I²C response or successful I²S setup does not prove a sensor's measurements are accurate or that speaker output is electrically safe.

The touch confirmation beep is generated by the ESP32 and sent over I²S to the MAX98357A (BCLK GPIO 26, LRC GPIO 25, DIN GPIO 27), then to the speaker. It is played at low gain and skipped while Azan or Quran audio is active.

If the amplifier or its wiring sparks, disconnect its power and the speaker; do not continue testing or assume that disconnecting SD resolves the fault. Check for shorts, confirm the board's VIN rating and speaker wiring, and keep both speaker terminals isolated from ground and each other.

### Wiring Tips
- Keep **I2C** lines short (under 10 cm) to preserve signal integrity.
- Use **common ground** for all peripherals; a single GND rail on a small breadboard works well.
- The I2S pins (BCLK, LRC, DIN) must be routed together to minimise skew – keep the audio wires as short as possible.
- The INMP441 uses a **separate I2S port** (I2S_NUM_0) from the amplifier (I2S_NUM_1) – they do **not** share pins.
- The firmware debounces the touch sensor in software.
- The touch input is read as a digital signal; use the TTP223B's default active-HIGH output mode.
- Shake detection uses the **MPU6050 accelerometer** on the shared I2C bus – no separate vibration sensor is needed.

### Visual Summary (ASCII schematic)
```
 ESP32 DevKit                OLED (I2C)           MAX98357A (I2S)
+----------+              +-----------+        +----------------+
|          |              |           |        |                |
|  GPIO21  +-- I2C SDA ---+ SDA       |        | BCLK  <-- GPIO26
|  GPIO22  +-- I2C SCL ---+ SCL       |        | LRC   <-- GPIO25
|          |              | VCC 3.3V  |        | DIN   <-- GPIO27
|          |              | GND       |        | VIN 3.3V       |
|          |              +-----------+        | GND            |
|          |                                   +-------+--------+
|          |      MPU6050 (I2C shared)                 |
|          |     +----------+                    Speaker +/−
|  GPIO21  +-----+ SDA      |         HMC5883L (I2C)
|  GPIO22  +-----+ SCL      |        +----------------+
|          |     | VCC 3.3V |        | SDA  <-- GPIO21
|          |     | GND      |        | SCL  <-- GPIO22
|          |     +----------+        | DRDY  unconnected
|          |                         +----------------+
|          |                         INMP441 (I2S Mic)
|          |     | VCC 3.3V |        +----------------+
|          |     | GND      |        | SCK   <-- GPIO16
|          |     +----------+        | WS    <-- GPIO17
|          |                         | SD    --> GPIO18
|   GPIO4  +-- TTP223B OUT          | L/R   --> GND
|          |                         | VDD 3.3V       |
+----------+                         +----------------+
```

### Pin Summary
- **I2C Bus**: GPIO 21 (SDA) / GPIO 22 (SCL) – shared by OLED, MPU6050, and HMC5883L compass.
- **I2S Amplifier**: GPIO 26 (BCLK) / GPIO 25 (LRC) / GPIO 27 (DIN).
- **I2S Microphone**: GPIO 16 (SCK) / GPIO 17 (WS) / GPIO 18 (SD).
- **Touch Sensor**: GPIO 4 (single TTP223B).
- **Power**: 5 V via USB‑C, 3.3 V regulator for logic.

Follow this map when assembling your hardware on a breadboard or PCB. Firmware support is implemented for the OLED, MPU6050, HMC5883L, and QMC5883L compass.
