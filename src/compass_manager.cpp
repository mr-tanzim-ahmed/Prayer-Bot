#include "compass_manager.h"
#include "config.h"
#include "i2c_bus.h"
#include "storage_manager.h"
#include <ArduinoJson.h>
#include <Wire.h>
#include <math.h>

namespace {
    enum class SensorType : uint8_t {
        NONE,
        HMC5883L,
        QMC5883L
    };

    constexpr uint8_t HMC5883L_ADDRESS = 0x1E;
    constexpr uint8_t QMC5883L_ADDRESS = 0x0D;
    constexpr uint8_t REG_CONFIG_A = 0x00;
    constexpr uint8_t REG_CONFIG_B = 0x01;
    constexpr uint8_t REG_MODE = 0x02;
    constexpr uint8_t REG_DATA_X_MSB = 0x03;
    constexpr uint8_t REG_STATUS = 0x09;
    constexpr uint8_t REG_ID_A = 0x0A;
    constexpr uint8_t STATUS_READY = 0x01;
    constexpr uint8_t QMC_REG_STATUS = 0x06;
    constexpr uint8_t QMC_REG_DATA_X_LSB = 0x00;
    constexpr uint8_t QMC_REG_CONTROL_1 = 0x09;
    constexpr uint8_t QMC_REG_CONTROL_2 = 0x0A;
    constexpr uint8_t QMC_REG_SET_RESET_PERIOD = 0x0B;
    constexpr uint8_t QMC_STATUS_OVERFLOW = 0x02;
    constexpr int16_t OVERFLOW_VALUE = -4096;
    constexpr uint16_t CALIBRATION_MIN_SAMPLES = 80;
    constexpr int16_t CALIBRATION_MIN_AXIS_SPAN = 100;
    constexpr uint32_t SENSOR_POLL_INTERVAL_MS = 50;
    constexpr uint32_t SENSOR_RETRY_MS = 5000;
    constexpr uint32_t SENSOR_STALE_MS = 1500;
    constexpr char CALIBRATION_PATH[] = "/compass_cal.json";

    bool sensorAvailable = false;
    SensorType sensorType = SensorType::NONE;
    bool calibrated = false;
    bool calibrationSaveFailed = false;
    uint16_t calibrationSamples = 0;
    int16_t minimumX = INT16_MAX;
    int16_t maximumX = INT16_MIN;
    int16_t minimumY = INT16_MAX;
    int16_t maximumY = INT16_MIN;
    float offsetX = 0.0f;
    float offsetY = 0.0f;
    float headingDegrees = 0.0f;
    bool headingValid = false;
    uint32_t lastSampleMs = 0;
    uint32_t lastReadAttemptMs = 0;
    uint32_t lastInitAttemptMs = 0;
    uint32_t lastRetryMs = 0;

    bool readRegisters(uint8_t address, uint8_t reg,
                       uint8_t* data, size_t length) {
        if (!data || length == 0 || length > 6 || !I2CBus::lock()) return false;

        Wire.beginTransmission(address);
        Wire.write(reg);
        bool success = Wire.endTransmission(false) == 0;
        if (success) {
            const size_t received = Wire.requestFrom(
                address,
                static_cast<uint8_t>(length));
            success = received == length;
            for (size_t i = 0; success && i < length; ++i) {
                const int value = Wire.read();
                if (value < 0) {
                    success = false;
                    break;
                }
                data[i] = static_cast<uint8_t>(value);
            }
        }
        I2CBus::unlock();
        return success;
    }

    bool writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
        if (!I2CBus::lock()) return false;
        Wire.beginTransmission(address);
        Wire.write(reg);
        Wire.write(value);
        const bool success = Wire.endTransmission() == 0;
        I2CBus::unlock();
        return success;
    }

    bool configureSensor() {
        uint8_t id[3] = {};
        if (readRegisters(HMC5883L_ADDRESS, REG_ID_A, id, sizeof(id)) &&
            id[0] == 'H' && id[1] == '4' && id[2] == '3') {
            if (!writeRegister(HMC5883L_ADDRESS, REG_CONFIG_A, 0x70) ||
                !writeRegister(HMC5883L_ADDRESS, REG_CONFIG_B, 0x20) ||
                !writeRegister(HMC5883L_ADDRESS, REG_MODE, 0x00)) {
                Serial.println("[COMPASS] HMC5883L configuration failed.");
                return false;
            }
            sensorType = SensorType::HMC5883L;
            Serial.println("[COMPASS] HMC5883L initialized at 0x1E.");
            return true;
        }

        uint8_t probe = 0;
        if (!readRegisters(QMC5883L_ADDRESS, QMC_REG_STATUS, &probe, 1)) {
            sensorType = SensorType::NONE;
            Serial.println("[COMPASS] No HMC5883L (0x1E) or QMC5883L (0x0D) detected.");
            return false;
        }
        if (!writeRegister(QMC5883L_ADDRESS, QMC_REG_CONTROL_2, 0x80) ||
            !writeRegister(QMC5883L_ADDRESS, QMC_REG_SET_RESET_PERIOD, 0x01) ||
            !writeRegister(QMC5883L_ADDRESS, QMC_REG_CONTROL_1, 0x1D)) {
            sensorType = SensorType::NONE;
            Serial.println("[COMPASS] QMC5883L configuration failed at 0x0D.");
            return false;
        }
        delay(10);
        uint8_t control = 0;
        if (!readRegisters(QMC5883L_ADDRESS, QMC_REG_CONTROL_1,
                           &control, 1) || control != 0x1D) {
            sensorType = SensorType::NONE;
            Serial.println("[COMPASS] QMC5883L configuration verification failed.");
            return false;
        }
        sensorType = SensorType::QMC5883L;
        Serial.println("[COMPASS] QMC5883L initialized at 0x0D.");
        return true;
    }

    bool loadCalibration() {
        if (!StorageManager::exists(CALIBRATION_PATH)) return false;

        const String json = StorageManager::readFile(CALIBRATION_PATH);
        JsonDocument document;
        if (json.isEmpty() || deserializeJson(document, json) ||
            (!document["x"].is<float>() && !document["x"].is<int>()) ||
            (!document["y"].is<float>() && !document["y"].is<int>())) {
            Serial.println("[COMPASS] Saved calibration is invalid; recalibration required.");
            return false;
        }

        const char* savedSensor = document["sensor"] | "";
        const char* currentSensor =
            sensorType == SensorType::QMC5883L ? "QMC5883L" : "HMC5883L";
        if ((sensorType == SensorType::QMC5883L &&
             strcmp(savedSensor, currentSensor) != 0) ||
            (savedSensor[0] != '\0' &&
             strcmp(savedSensor, currentSensor) != 0)) {
            Serial.println("[COMPASS] Calibration is for a different sensor; recalibration required.");
            return false;
        }

        const float savedX = document["x"].as<float>();
        const float savedY = document["y"].as<float>();
        if (!isfinite(savedX) || !isfinite(savedY) ||
            fabsf(savedX) > 32768.0f || fabsf(savedY) > 32768.0f) {
            Serial.println("[COMPASS] Saved calibration is outside valid range.");
            return false;
        }
        offsetX = savedX;
        offsetY = savedY;
        return true;
    }

    void saveCalibration() {
        JsonDocument document;
        document["x"] = offsetX;
        document["y"] = offsetY;
        document["sensor"] = sensorType == SensorType::QMC5883L
            ? "QMC5883L" : "HMC5883L";
        String json;
        if (serializeJson(document, json) == 0 || json.isEmpty() ||
            !StorageManager::writeFile(CALIBRATION_PATH, json)) {
            calibrationSaveFailed = true;
            Serial.println("[COMPASS] Could not persist calibration; using it for this session.");
            return;
        }
        calibrationSaveFailed = false;
        Serial.println("[COMPASS] Hard-iron calibration saved.");
    }

    int16_t toInt16(uint8_t high, uint8_t low) {
        return static_cast<int16_t>(
            (static_cast<uint16_t>(high) << 8) | low);
    }

    void collectCalibration(int16_t x, int16_t y) {
        if (calibrated) return;
        minimumX = min(minimumX, x);
        maximumX = max(maximumX, x);
        minimumY = min(minimumY, y);
        maximumY = max(maximumY, y);
        if (calibrationSamples < UINT16_MAX) ++calibrationSamples;

        if (calibrationSamples < CALIBRATION_MIN_SAMPLES ||
            maximumX - minimumX < CALIBRATION_MIN_AXIS_SPAN ||
            maximumY - minimumY < CALIBRATION_MIN_AXIS_SPAN) {
            return;
        }

        offsetX = (static_cast<float>(maximumX) + minimumX) * 0.5f;
        offsetY = (static_cast<float>(maximumY) + minimumY) * 0.5f;
        if (!isfinite(offsetX) || !isfinite(offsetY)) {
            offsetX = 0.0f;
            offsetY = 0.0f;
            return;
        }
        calibrated = true;
        saveCalibration();
    }

    void updateHeading(int16_t x, int16_t y) {
        collectCalibration(x, y);
        const float correctedX = static_cast<float>(x) - offsetX;
        const float correctedY = static_cast<float>(y) - offsetY;
        if (!isfinite(correctedX) || !isfinite(correctedY) ||
            (correctedX == 0.0f && correctedY == 0.0f)) {
            headingValid = false;
            return;
        }

        float heading = atan2f(correctedX, correctedY) * 180.0f / PI;
        if (!isfinite(heading)) {
            headingValid = false;
            return;
        }
        if (heading < 0.0f) heading += 360.0f;
        headingDegrees = heading;
        headingValid = true;
        lastSampleMs = millis();
    }

    bool readSample(int16_t& x, int16_t& y, int16_t& z) {
        uint8_t status = 0;
        const uint8_t address = sensorType == SensorType::QMC5883L
            ? QMC5883L_ADDRESS : HMC5883L_ADDRESS;
        const uint8_t statusRegister = sensorType == SensorType::QMC5883L
            ? QMC_REG_STATUS : REG_STATUS;
        if (!readRegisters(address, statusRegister, &status, 1)) return false;
        if ((status & STATUS_READY) == 0) return false;
        if (sensorType == SensorType::QMC5883L &&
            (status & QMC_STATUS_OVERFLOW) != 0) {
            Serial.println("[COMPASS] QMC5883L overflow; discard sample.");
            return false;
        }

        uint8_t data[6] = {};
        const uint8_t dataRegister =
            sensorType == SensorType::QMC5883L
                ? QMC_REG_DATA_X_LSB : REG_DATA_X_MSB;
        if (!readRegisters(address, dataRegister, data, sizeof(data)))
            return false;
        if (sensorType == SensorType::QMC5883L) {
            x = toInt16(data[1], data[0]);
            y = toInt16(data[3], data[2]);
            z = toInt16(data[5], data[4]);
        } else {
            x = toInt16(data[0], data[1]);
            z = toInt16(data[2], data[3]);
            y = toInt16(data[4], data[5]);
        }
        if (sensorType == SensorType::HMC5883L &&
            (x == OVERFLOW_VALUE || y == OVERFLOW_VALUE ||
             z == OVERFLOW_VALUE)) {
            Serial.println("[COMPASS] Magnetometer overflow; discard sample.");
            return false;
        }
        return true;
    }
}

namespace CompassManager {

bool init() {
    if (!I2CBus::init()) {
        Serial.println("[COMPASS] Could not initialize shared I2C bus lock.");
        sensorAvailable = false;
        return false;
    }
    lastInitAttemptMs = millis();
    sensorAvailable = configureSensor();
    headingValid = false;
    if (sensorAvailable && loadCalibration()) {
        calibrated = true;
        Serial.println("[COMPASS] Saved calibration loaded.");
    } else {
        calibrated = false;
        calibrationSamples = 0;
        minimumX = INT16_MAX;
        maximumX = INT16_MIN;
        minimumY = INT16_MAX;
        maximumY = INT16_MIN;
    }
    return sensorAvailable;
}

void update() {
    const uint32_t now = millis();
    if (!sensorAvailable) {
        if (now - lastInitAttemptMs >= SENSOR_RETRY_MS) init();
        return;
    }
    if (now - lastReadAttemptMs < SENSOR_POLL_INTERVAL_MS) return;
    lastReadAttemptMs = now;

    int16_t x, y, z;
    if (!readSample(x, y, z)) {
        if (lastSampleMs == 0 || now - lastSampleMs > SENSOR_STALE_MS) {
            headingValid = false;
        }
        if (now - lastRetryMs >= SENSOR_RETRY_MS) {
            lastRetryMs = now;
            uint8_t probe = 0;
            const uint8_t address = sensorType == SensorType::QMC5883L
                ? QMC5883L_ADDRESS : HMC5883L_ADDRESS;
            const uint8_t reg = sensorType == SensorType::QMC5883L
                ? QMC_REG_STATUS : REG_ID_A;
            if (!readRegisters(address, reg, &probe, 1)) {
                Serial.println("[COMPASS] Sensor communication lost; will retry.");
                sensorAvailable = false;
                sensorType = SensorType::NONE;
                headingValid = false;
            }
        }
        return;
    }
    updateHeading(x, y);
}

bool resetCalibration() {
    if (!StorageManager::removeFile(CALIBRATION_PATH)) {
        Serial.println("[COMPASS] Could not remove saved calibration.");
        calibrationSaveFailed = true;
        return false;
    }
    calibrated = false;
    calibrationSaveFailed = false;
    calibrationSamples = 0;
    minimumX = INT16_MAX;
    maximumX = INT16_MIN;
    minimumY = INT16_MAX;
    maximumY = INT16_MIN;
    offsetX = 0.0f;
    offsetY = 0.0f;
    headingValid = false;
    Serial.println("[COMPASS] Calibration reset; rotate the device flat through 360 degrees.");
    return true;
}

void getReading(CompassReading& reading) {
    reading.headingDegrees = headingDegrees;
    reading.headingValid = headingValid &&
        millis() - lastSampleMs <= SENSOR_STALE_MS;
    reading.sensorAvailable = sensorAvailable;
    reading.calibrated = calibrated;
    reading.calibrationSaveFailed = calibrationSaveFailed;
    reading.calibrationSamples = calibrationSamples;
}

}
