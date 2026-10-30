#include "shake_detector.h"
#include "config.h"
#include <Wire.h>
#include "i2c_bus.h"

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
    unsigned long lastMpuCheckMs = 0;
    constexpr unsigned long MPU_RETRY_INTERVAL_MS = 5000UL;

    bool writeMPU(uint8_t reg, uint8_t val) {
        if (!I2CBus::lock()) return false;
        Wire.beginTransmission(MPU6050_I2C_ADDRESS);
        Wire.write(reg);
        Wire.write(val);
        const bool success = Wire.endTransmission() == 0;
        I2CBus::unlock();
        return success;
    }

    bool readAxis(uint8_t regH, int16_t& value) {
        if (!I2CBus::lock()) return false;
        Wire.beginTransmission(MPU6050_I2C_ADDRESS);
        Wire.write(regH);
        if (Wire.endTransmission(false) != 0 ||
            Wire.requestFrom((uint8_t)MPU6050_I2C_ADDRESS, (uint8_t)2) != 2 ||
            Wire.available() < 2) {
            I2CBus::unlock();
            return false;
        }

        const int highByte = Wire.read();
        const int lowByte = Wire.read();
        I2CBus::unlock();
        if (highByte < 0 || lowByte < 0) return false;

        value = (int16_t)((highByte << 8) | lowByte);
        return true;
    }
}

namespace ShakeDetector {

bool init() {
    lastMpuCheckMs = millis();

    // Check MPU6050 presence
    if (!I2CBus::lock()) {
        Serial.println("[MPU] Could not lock I2C bus.");
        mpuAvailable = false;
        return false;
    }
    Wire.beginTransmission(MPU6050_I2C_ADDRESS);
    const bool present = Wire.endTransmission() == 0;
    I2CBus::unlock();
    if (!present) {
        Serial.println("[MPU] Not found at 0x68");
        mpuAvailable = false;
        return false;
    }

    // Wake up MPU6050 (clear sleep bit)
    if (!writeMPU(0x6B, 0x00)) {
        Serial.println("[MPU] Failed to wake sensor.");
        mpuAvailable = false;
        return false;
    }
    delay(10);

    // Set accelerometer to +/- 8g range
    if (!writeMPU(0x1C, 0x10)) {
        Serial.println("[MPU] Failed to configure accelerometer.");
        mpuAvailable = false;
        return false;
    }

    mpuAvailable = true;
    Serial.println("[MPU] Initialized.");
    return true;
}

void update() {
    if (!mpuAvailable) {
        if (millis() - lastMpuCheckMs >= MPU_RETRY_INTERVAL_MS) init();
        return;
    }

    int16_t ax;
    int16_t ay;
    int16_t az;
    if (!readAxis(0x3B, ax) ||
        !readAxis(0x3D, ay) ||
        !readAxis(0x3F, az)) {
        mpuAvailable = false;
        lastMpuCheckMs = millis();
        peakCount = 0;
        Serial.println("[MPU] Read failed; sensor will be retried.");
        return;
    }

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
