#include "screens/qibla_screen.h"
#include "screens/screen_ui.h"
#include "managers/compass_manager.h"
#include "core/config.h"
#include "managers/display_manager.h"
#include <math.h>
#include <stdio.h>

namespace {
    constexpr double KAABA_LATITUDE = 21.4225;
    constexpr double KAABA_LONGITUDE = 39.8262;
    constexpr float RADIANS_TO_DEGREES = 57.2957795131f;

    bool getQiblaBearing(float latitude, float longitude, float& bearing) {
        if (!isfinite(latitude) || !isfinite(longitude) ||
            latitude < -90.0f || latitude > 90.0f ||
            longitude < -180.0f || longitude > 180.0f) {
            return false;
        }

        const double lat = latitude * (PI / 180.0);
        const double kaabaLat = KAABA_LATITUDE * (PI / 180.0);
        const double deltaLon =
            (KAABA_LONGITUDE - longitude) * (PI / 180.0);
        const double y = sin(deltaLon) * cos(kaabaLat);
        const double x = cos(lat) * sin(kaabaLat) -
                         sin(lat) * cos(kaabaLat) * cos(deltaLon);
        if (!isfinite(x) || !isfinite(y) ||
            (fabs(x) < 1e-12 && fabs(y) < 1e-12)) {
            return false;
        }

        double result = atan2(y, x) * RADIANS_TO_DEGREES;
        if (!isfinite(result)) return false;
        if (result < 0.0) result += 360.0;
        bearing = static_cast<float>(result);
        return true;
    }

    void drawQiblaArrow(U8G2& oled, float relativeAngle, int centerX,
                        int centerY, float scale);

    void drawQiblaHeader(U8G2& oled, const char* bearingTag) {
        drawQiblaArrow(oled, 0.0f, 8, 8, 0.35f);
        oled.setFont(u8g2_font_5x8_tf);
        oled.drawStr(18, 10, "QIBLA");

        if (bearingTag) {
            oled.setFont(u8g2_font_4x6_tf);
            const int width = oled.getStrWidth(bearingTag);
            oled.drawStr(SCREEN_WIDTH - width - 3, 9, bearingTag);
        }
    }

    void drawQiblaArrow(U8G2& oled, float relativeAngle, int centerX,
                        int centerY, float scale) {
        constexpr float pivotX = 24.0f;
        constexpr float pivotY = 39.0f;
        const float radians = relativeAngle * (PI / 180.0f);
        const float cosine = cosf(radians);
        const float sine = sinf(radians);
        const auto transform = [&](float x, float y, int& screenX,
                                   int& screenY) {
            const float dx = x - pivotX;
            const float dy = y - pivotY;
            screenX = static_cast<int>(roundf(
                centerX + (dx * cosine - dy * sine) * scale));
            screenY = static_cast<int>(roundf(
                centerY + (dx * sine + dy * cosine) * scale));
        };

        int tipX, tipY, notchX, notchY, rightX, rightY, leftX, leftY;
        transform(24.0f, 22.0f, tipX, tipY);
        transform(24.0f, 43.0f, notchX, notchY);
        transform(37.0f, 55.0f, rightX, rightY);
        transform(11.0f, 55.0f, leftX, leftY);

        oled.drawTriangle(tipX, tipY, rightX, rightY, notchX, notchY);
        oled.drawTriangle(tipX, tipY, notchX, notchY, leftX, leftY);
    }

    void drawCompassScreen(const CompassReading& reading,
                           bool locationValid, float qiblaBearing) {
        U8G2& oled = DisplayManager::getDisplay();
        DisplayManager::clearBuffer();

        char tagStr[16] = {};
        if (locationValid) {
            snprintf(tagStr, sizeof(tagStr), "%d\xb0 Makkah", static_cast<int>(roundf(qiblaBearing)));
        }
        drawQiblaHeader(oled, tagStr[0] ? tagStr : nullptr);

        const bool headingReady = reading.sensorAvailable &&
            reading.calibrated && reading.headingValid && locationValid;
        if (headingReady) {
            float relAngle = qiblaBearing - reading.headingDegrees;
            while (relAngle > 180.0f) relAngle -= 360.0f;
            while (relAngle < -180.0f) relAngle += 360.0f;

            drawQiblaArrow(oled, relAngle, SCREEN_WIDTH / 2, 34, 1.0f);

            oled.setFont(u8g2_font_5x8_tf);
            if (fabsf(relAngle) <= 6.0f) {
                DisplayManager::drawCenteredText(">> FACING KAABA <<", 62);
            } else if (relAngle > 6.0f) {
                char hint[24];
                snprintf(hint, sizeof(hint), "Turn Right %d\xb0 >", static_cast<int>(roundf(relAngle)));
                DisplayManager::drawCenteredText(hint, 62);
            } else {
                char hint[24];
                snprintf(hint, sizeof(hint), "< Turn Left %d\xb0", static_cast<int>(roundf(-relAngle)));
                DisplayManager::drawCenteredText(hint, 62);
            }
        } else {
            ScreenUI::drawCard(oled, 4, 18, SCREEN_WIDTH - 8, 42);
            const char* status = "WAITING FOR COMPASS";
            if (!reading.sensorAvailable) {
                status = "COMPASS NOT FOUND";
            } else if (!locationValid) {
                status = "LOCATION NOT SET";
            } else if (!reading.calibrated) {
                status = "ROTATE TO CALIBRATE";
            }
            oled.setFont(u8g2_font_6x10_tf);
            DisplayManager::drawCenteredText(status, 38);
            oled.setFont(u8g2_font_5x8_tf);
            DisplayManager::drawCenteredText(reading.calibrated ? "Keep flat & rotate" : "Move in figure-8", 50);
        }
        DisplayManager::sendBuffer();
    }
}

namespace QiblaScreen {

void draw(float latitude, float longitude) {
    CompassManager::update();
    CompassReading reading = {};
    CompassManager::getReading(reading);

    float qiblaBearing = 0.0f;
    const bool locationValid =
        getQiblaBearing(latitude, longitude, qiblaBearing);
    drawCompassScreen(reading, locationValid, qiblaBearing);
}

}
