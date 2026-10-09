# Hardware Connections for Prayer‑Bot

This document describes the GPIO wiring between the **ESP32‑S3 (DevKitC‑1)** and all peripheral modules used in the project.

| Peripheral | ESP32‑S3 Pin | Function | Notes |
|------------|--------------|----------|-------|
| **OLED Display (I2C, 128×64)** | **GPIO8** | SDA | Connect to the display **SDA** line. Use a 4.7 kΩ pull‑up resistor to 3.3 V (already on many modules). |
| | **GPIO9** | SCL | Connect to the display **SCL** line. 4.7 kΩ pull‑up to 3.3 V. |
| | **3.3 V** | VCC | Power the OLED. |
| | **GND** | GND | Common ground. |
| **MAX98357A I2S Amplifier** | **GPIO47** | BCLK (I2S Bit Clock) | I2S0 BCK – drives the audio clock. |
| | **GPIO21** | LRCLK (I2S Word Select) | I2S0 LRCK – also called WS. |
| | **GPIO18** | DIN (I2S Data In) | Audio data sent from ESP32 to the amp. |
| | **GPIO19** | **SD** (Shutdown) | Pull‑low to enable the amp, high to mute (optional). |
| | **3.3 V** | VCC | Supply the amp (max 3.3 V). |
| | **GND** | GND | Common ground. |
| **Vibration / Shake Sensor (SW‑420)** | **GPIO2** | INPUT | Connect the sensor’s **OUT** to GPIO2. Enable the internal pull‑up (`pinMode(2, INPUT_PULLUP)`). |
| | **3.3 V** | VCC | Power the sensor. |
| | **GND** | GND | Common ground. |
| **Push Button (Menu / Pomodoro)** | **GPIO0** | INPUT | Connect one side of the button to GPIO0 and the other to GND. Use `INPUT_PULLUP` to avoid external resistors. |
| | **3.3 V** (optional) | VCC | If you prefer active‑high logic, wire to 3.3 V and enable `INPUT`. |
| **INMP441 I2S Microphone (optional)** | **GPIO47** | BCLK | Shares the same BCLK as the audio amp (I2S0 BCK). |
| | **GPIO21** | LRCLK | Shares the same LRCLK as the amp (I2S0 LRCK). |
| | **GPIO20** | DOUT (I2S Data Out) | Connect microphone data to GPIO20. |
| | **3.3 V** | VCC | Power the microphone (max 3.3 V). |
| | **GND** | GND | Common ground. |
| **Power Supply** | **USB‑C / VIN** | 5 V | Provide 5 V via the board’s USB‑C connector or a regulated 5 V supply. |
| | **3.3 V** regulator | 3.3 V | Internally generated for logic. |
| | **GND** | Ground | All components share this ground. |

### Wiring Tips
- Keep **I2C** lines short (under 10 cm) to preserve signal integrity.
- Use **common ground** for all peripherals; a single GND rail on a small breadboard works well.
- The I2S pins (BCLK, LRCLK, DIN) must be routed together to minimise skew – keep the audio wires as short as possible.
- If you enable the optional microphone, ensure its **BCLK/LRCLK** are tied to the same pins used by the amplifier; the firmware can switch between audio output and input.
- Debounce the push‑button in software (`delay(50)` or use a hardware RC filter) to avoid multiple state changes.
- For the vibration sensor, a simple digital read is sufficient – the sensor outputs HIGH when motion is detected.

### Visual Summary (ASCII schematic)
```
 ESP32‑S3                  OLED (I2C)          MAX98357A (I2S)          Vibration
+----------+            +-----------+      +----------------+        Sensor
|          |            |   SDA 8   |      | BCLK 47  ----+------> OUT
|   GPIO8  +------------+-----------+      | LRCLK 21 ----+------> OUT
|          |            |   SCL 9   |      | DIN 18  <----+------> IN
|   GPIO9  +------------+-----------+      | SD   19  <----+ (optional)
|          |            |   VCC 3.3V|      | VCC 3.3V      |
|   3.3V   +------------+-----------+      | GND   GND      |
|   GND    +------------+-----------+      +----------------+
|   GPIO2  +----------------------------> Vibration OUT (HIGH on shake)
|   GPIO0  +----------------------------> Button (pulled‑up)
|   GPIO47 +----------------------------> I2S BCLK (shared with amp & mic)
|   GPIO21 +----------------------------> I2S LRCLK (shared)
|   GPIO18 +----------------------------> I2S DIN (audio to amp)
|   GPIO20 +----------------------------> I2S DOUT (mic optional)
+----------+                                              |
                                                          +--- GND
```

### Summary
- **I2C**: GPIO8 (SDA) / GPIO9 (SCL)
- **I2S Audio**: GPIO47 (BCLK) / GPIO21 (LRCLK) / GPIO18 (DIN) – optional mic on GPIO20 (DOUT).
- **Sensors**: GPIO2 (vibration), GPIO0 (button).
- **Power**: 5 V via USB‑C, 3.3 V regulator for logic.

Follow this map when assembling your hardware on a breadboard or PCB, and the firmware will detect each peripheral automatically.
