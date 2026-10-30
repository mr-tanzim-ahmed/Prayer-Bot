#include "qibla_screen.h"
#include "screen_ui.h"
#include "../compass_manager.h"
#include "../config.h"
#include "../display_manager.h"
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

    void drawQiblaArrow(U8G2& oled, float relativeAngle) {
        constexpr int centerX = SCREEN_WIDTH / 2;
        constexpr int centerY = 36;
        constexpr int ringRadius = 18;

        // Draw compass ring with 4 cardinal tick marks
        oled.drawCircle(centerX, centerY, ringRadius);
        oled.drawVLine(centerX, centerY - ringRadius - 3, 3); // Forward / Device top
        oled.drawVLine(centerX, centerY + ringRadius, 3);
        oled.drawHLine(centerX - ringRadius - 3, centerY, 3);
        oled.drawHLine(centerX + ringRadius, centerY, 3);

        const float radians = relativeAngle * (PI / 180.0f);
        const float forwardX = sinf(radians);
        const float forwardY = -cosf(radians);
        const float sideX = cosf(radians);
        const float sideY = sinf(radians);

        // Arrow geometry (sleek directional navigation arrow -->)
        constexpr float tipDist = 17.0f;
        constexpr float wingDist = 7.0f;
        constexpr float wingWidth = 6.0f;
        constexpr float notchDist = 10.0f;
        constexpr float tailDist = 13.0f;
        constexpr float fletchDist = 4.0f;

        const float tipX = centerX + forwardX * tipDist;
        const float tipY = centerY + forwardY * tipDist;
        const float leftWingX = centerX + forwardX * wingDist + sideX * wingWidth;
        const float leftWingY = centerY + forwardY * wingDist + sideY * wingWidth;
        const float rightWingX = centerX + forwardX * wingDist - sideX * wingWidth;
        const float rightWingY = centerY + forwardY * wingDist - sideY * wingWidth;
        const float notchX = centerX + forwardX * notchDist;
        const float notchY = centerY + forwardY * notchDist;

        const float tailX = centerX - forwardX * tailDist;
        const float tailY = centerY - forwardY * tailDist;
        const float fletchLeftX = tailX + forwardX * fletchDist + sideX * fletchDist;
        const float fletchLeftY = tailY + forwardY * fletchDist + sideY * fletchDist;
        const float fletchRightX = tailX + forwardX * fletchDist - sideX * fletchDist;
        const float fletchRightY = tailY + forwardY * fletchDist - sideY * fletchDist;

        // Draw crisp filled chevron arrowhead
        oled.drawTriangle(static_cast<int>(roundf(tipX)), static_cast<int>(roundf(tipY)),
                          static_cast<int>(roundf(leftWingX)), static_cast<int>(roundf(leftWingY)),
                          static_cast<int>(roundf(notchX)), static_cast<int>(roundf(notchY)));
        oled.drawTriangle(static_cast<int>(roundf(tipX)), static_cast<int>(roundf(tipY)),
                          static_cast<int>(roundf(rightWingX)), static_cast<int>(roundf(rightWingY)),
                          static_cast<int>(roundf(notchX)), static_cast<int>(roundf(notchY)));

        // Draw sleek slim arrow shaft (2 pixels wide for crisp visibility without chunkiness)
        oled.drawLine(static_cast<int>(roundf(notchX)), static_cast<int>(roundf(notchY)),
                      static_cast<int>(roundf(tailX)), static_cast<int>(roundf(tailY)));
        oled.drawLine(static_cast<int>(roundf(notchX + sideX * 0.7f)), static_cast<int>(roundf(notchY + sideY * 0.7f)),
                      static_cast<int>(roundf(tailX + sideX * 0.7f)), static_cast<int>(roundf(tailY + sideY * 0.7f)));

        // Draw tail fletchings (the "-->" feathers)
        oled.drawLine(static_cast<int>(roundf(tailX)), static_cast<int>(roundf(tailY)),
                      static_cast<int>(roundf(fletchLeftX)), static_cast<int>(roundf(fletchLeftY)));
        oled.drawLine(static_cast<int>(roundf(tailX)), static_cast<int>(roundf(tailY)),
                      static_cast<int>(roundf(fletchRightX)), static_cast<int>(roundf(fletchRightY)));

        // Center pivot dot
        oled.drawDisc(centerX, centerY, 2);
    }

    void drawCompassScreen(const CompassReading& reading,
                           bool locationValid, float qiblaBearing) {
        U8G2& oled = DisplayManager::getDisplay();
        DisplayManager::clearBuffer();

        char tagStr[16] = {};
        if (locationValid) {
            snprintf(tagStr, sizeof(tagStr), "%d\xb0 Makkah", static_cast<int>(roundf(qiblaBearing)));
        }
        ScreenUI::drawHeader(oled, "QIBLA COMPASS", tagStr[0] ? tagStr : nullptr);

        const bool headingReady = reading.sensorAvailable &&
            reading.calibrated && reading.headingValid && locationValid;
        if (headingReady) {
            float relAngle = qiblaBearing - reading.headingDegrees;
            while (relAngle > 180.0f) relAngle -= 360.0f;
            while (relAngle < -180.0f) relAngle += 360.0f;

            drawQiblaArrow(oled, relAngle);

            oled.setFont(u8g2_font_5x8_tf);
            if (fabsf(relAngle) <= 6.0f) {
                // Aligned with Kaaba! Highlight with double circle and indicator
                oled.drawCircle(SCREEN_WIDTH / 2, 36, 15);
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
