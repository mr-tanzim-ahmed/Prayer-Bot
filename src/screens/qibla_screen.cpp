#include "screens/qibla_screen.h"
#include "screens/screen_ui.h"
#include "managers/device/compass_manager.h"
#include <prayer_bot_config.h>
#include "managers/device/display_manager.h"
#include <math.h>

namespace {
    constexpr double KAABA_LATITUDE = 21.4225;
    constexpr double KAABA_LONGITUDE = 39.8262;
    constexpr float RADIANS_TO_DEGREES = 57.2957795131f;
    constexpr float HEADING_FILTER_TIME_CONSTANT_MS = 160.0f;
    constexpr const char* CARDINAL_DIRECTIONS[] = {
        "N", "NE", "E", "SE", "S", "SW", "W", "NW"
    };

    bool headingFilterInitialized = false;
    float filteredHeadingDegrees = 0.0f;
    uint32_t lastHeadingFilterMs = 0;

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

    const char* getCardinalDirection(float bearing) {
        const int index = static_cast<int>(
            floorf((bearing + 22.5f) / 45.0f)) % 8;
        return CARDINAL_DIRECTIONS[index];
    }

    int roundedBearing(float bearing) {
        const int rounded = static_cast<int>(lroundf(bearing));
        return rounded >= 360 ? 0 : rounded;
    }

    float normalizeBearing(float bearing) {
        while (bearing < 0.0f) bearing += 360.0f;
        while (bearing >= 360.0f) bearing -= 360.0f;
        return bearing;
    }

    float smoothHeading(float heading, uint32_t now) {
        if (!headingFilterInitialized) {
            filteredHeadingDegrees = heading;
            lastHeadingFilterMs = now;
            headingFilterInitialized = true;
            return filteredHeadingDegrees;
        }

        const uint32_t elapsedMs = now - lastHeadingFilterMs;
        lastHeadingFilterMs = now;
        const float alpha = 1.0f - expf(
            -static_cast<float>(elapsedMs) / HEADING_FILTER_TIME_CONSTANT_MS);
        float difference = heading - filteredHeadingDegrees;
        while (difference > 180.0f) difference -= 360.0f;
        while (difference < -180.0f) difference += 360.0f;

        filteredHeadingDegrees += alpha * difference;
        if (filteredHeadingDegrees < 0.0f) filteredHeadingDegrees += 360.0f;
        if (filteredHeadingDegrees >= 360.0f) filteredHeadingDegrees -= 360.0f;
        return filteredHeadingDegrees;
    }

    void drawCompassIcon(U8G2& oled, float relativeAngle,
                         bool headingReady) {
        constexpr int frameX = 16;
        constexpr int frameY = 17;
        constexpr int frameWidth = 38;
        constexpr int frameHeight = 45;
        constexpr float centerX = 35.0f;
        constexpr float centerY = 40.0f;
        constexpr float iconScale = 0.6f;

        oled.drawRFrame(frameX, frameY, frameWidth, frameHeight, 5);
        oled.setDrawColor(1);
        oled.drawCircle(static_cast<int>(centerX),
                        static_cast<int>(centerY), 18);
        oled.drawCircle(static_cast<int>(centerX),
                        static_cast<int>(centerY), 17);

        const float radians = relativeAngle * (PI / 180.0f);
        const float cosine = cosf(radians);
        const float sine = sinf(radians);
        const auto transform = [&](float x, float y, int& screenX,
                                   int& screenY) {
            const float dx = x * iconScale;
            const float dy = y * iconScale;
            screenX = static_cast<int>(roundf(
                centerX + dx * cosine - dy * sine));
            screenY = static_cast<int>(roundf(
                centerY + dx * sine + dy * cosine));
        };

        if (headingReady) {
            int tipX, tipY, centerTipX, centerTipY, rightX, rightY;
            int leftX, leftY;
            transform(0.0f, -18.0f, tipX, tipY);
            transform(0.0f, 6.0f, centerTipX, centerTipY);
            transform(14.0f, 18.0f, rightX, rightY);
            transform(-14.0f, 18.0f, leftX, leftY);

            oled.drawTriangle(tipX, tipY, rightX, rightY,
                              centerTipX, centerTipY);
            oled.drawTriangle(tipX, tipY, centerTipX, centerTipY,
                              leftX, leftY);
        }
        oled.setDrawColor(1);
    }

    void drawCompassScreen(const CompassReading& reading,
                           bool locationValid, float qiblaBearing) {
        U8G2& oled = DisplayManager::getDisplay();
        DisplayManager::clearBuffer();
        ScreenUI::drawHeader(oled, "QIBLA");

        const bool headingReady = reading.sensorAvailable &&
            reading.calibrated && reading.headingValid && locationValid;
        float relativeBearing = qiblaBearing;
        if (headingReady) {
            const float displayHeading = smoothHeading(
                reading.headingDegrees, millis());
            relativeBearing = normalizeBearing(qiblaBearing - displayHeading);
            float relAngle = relativeBearing;
            if (relAngle > 180.0f) relAngle -= 360.0f;
            drawCompassIcon(oled, relAngle, true);
        } else {
            headingFilterInitialized = false;
            drawCompassIcon(oled, 0.0f, false);
        }

        if (headingReady) {
            char bearingText[20];
            snprintf(bearingText, sizeof(bearingText), "%s %03d\xC2\xB0",
                     getCardinalDirection(relativeBearing),
                     roundedBearing(relativeBearing));
            oled.setFont(u8g2_font_7x14B_tf);
            const int textWidth = oled.getUTF8Width(bearingText);
            oled.drawUTF8(67 + (SCREEN_WIDTH - 67 - textWidth) / 2,
                          42, bearingText);
        }
        if (reading.sensorAvailable && !reading.calibrated) {
            oled.setFont(u8g2_font_4x6_tf);
            oled.drawStr(67, 52, "TURN 360 SLOW");
            char progress[20];
            snprintf(progress, sizeof(progress), "LEVEL DIRS %u/8",
                     static_cast<unsigned>(reading.calibrationDirections));
            oled.drawStr(67, 61, progress);
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
