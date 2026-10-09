#include "shake_detector.h"
#include "config.h"
#include <Wire.h>

// ============================================================
//  SHAKE DETECTOR IMPLEMENTATION
//  Reads MPU6050 raw accelerometer data and detects
//  shake gestures (multiple peaks within a time window).
// ============================================================

namespace {
    bool shakeDetected = false;
    bool mpuAvailable = false;

    uint8_t peakCount = 0;
    unsigned long firstPeakTime = 0;

    void writeMPU(uint8_t reg, uint8_t val) {
        Wire.beginTransmission(MPU6050_I2C_ADDRESS);
        Wire.write(reg);
        Wire.write(val);
        Wire.endTransmission();
    }

    int16_t readAxis(uint8_t regH) {
        Wire.beginTransmission(MPU6050_I2C_ADDRESS);
        Wire.write(regH);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)MPU6050_I2C_ADDRESS, (uint8_t)2);
        int16_t val = (Wire.read() << 8) | Wire.read();
        return val;
    }
}

namespace ShakeDetector {

bool init() {
    // Check MPU6050 presence
    Wire.beginTransmission(MPU6050_I2C_ADDRESS);
    if (Wire.endTransmission() != 0) {
        Serial.println("[MPU] Not found at 0x68");
        mpuAvailable = false;
        return false;
    }

    // Wake up MPU6050 (clear sleep bit)
    writeMPU(0x6B, 0x00);
    delay(10);

    // Set accelerometer to +/- 8g range
    writeMPU(0x1C, 0x10);

    mpuAvailable = true;
    Serial.println("[MPU] Initialized.");
    return true;
}

void update() {
    if (!mpuAvailable) return;

    int16_t ax = readAxis(0x3B);
    int16_t ay = readAxis(0x3D);
    int16_t az = readAxis(0x3F);

    // Calculate magnitude (squared to avoid sqrt)
    int32_t mag = (int32_t)ax * ax + (int32_t)ay * ay + (int32_t)az * az;
    int32_t threshold = (int32_t)SHAKE_THRESHOLD * SHAKE_THRESHOLD;

    unsigned long now = millis();

    if (mag > threshold) {
        if (peakCount == 0) {
            firstPeakTime = now;
        }
        peakCount++;

        if (peakCount >= SHAKE_COUNT_REQUIRED &&
            (now - firstPeakTime) <= SHAKE_WINDOW_MS) {
            shakeDetected = true;
            peakCount = 0;
        }
    }

    // Reset window if expired
    if (peakCount > 0 && (now - firstPeakTime) > SHAKE_WINDOW_MS) {
        peakCount = 0;
    }
}

bool isShakeDetected() {
    if (shakeDetected) {
        shakeDetected = false;
        return true;
    }
    return false;
}

void reset() {
    shakeDetected = false;
    peakCount = 0;
}

} // namespace ShakeDetector
