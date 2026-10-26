#include "servos.h"
#include <Arduino.h>
#include <ESP32Servo.h>
#include "config.h"
#include "state.h"
#include "utils.h"

Servo servoRotate;
Servo servoGrip;
uint32_t lastServoUpdateMs = 0;
float servoRotateFilteredDeg = HOME_TOOL_ROTATE_DEG;
float servoGripFilteredDeg = HOME_GRIP_DEG;

int angleToPulseUs(float angleDeg, int minUs, int maxUs)
{
    const float a = clampFloat(angleDeg, 0.0f, 180.0f);
    return lroundf(minUs + (a / 180.0f) * (maxUs - minUs));
}

void updateServoTargets(float rotateCmd, float gripCmd, float dt)
{
    // Плавний розгін/гальмування команди схвата (без різких стрибків швидкості)
    static float gripCmdSmoothed = 0.0f;
    const float k = clampFloat(dt / GRIP_RAMP_S, 0.0f, 1.0f);
    gripCmdSmoothed += (gripCmd - gripCmdSmoothed) * k;
    if (gripCmd == 0.0f && fabsf(gripCmdSmoothed) < 0.01f) gripCmdSmoothed = 0.0f;

    robot.toolRotateDeg += rotateCmd * TOOL_ROTATE_SPEED_DEG_S * dt;
    robot.gripperDeg += gripCmdSmoothed * GRIP_SPEED_DEG_S * dt;
    robot.toolRotateDeg = clampFloat(robot.toolRotateDeg, TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
    robot.gripperDeg = clampFloat(robot.gripperDeg, GRIP_MIN_DEG, GRIP_MAX_DEG);
}

void refreshServos()
{
    if (robot.estopLatched) return;

    const uint32_t now = millis();
    if (now - lastServoUpdateMs < SERVO_PERIOD_MS) return;
    const float servoDtS = clampFloat(static_cast<float>(now - lastServoUpdateMs) / 1000.0f, 0.0f, 0.1f);
    lastServoUpdateMs = now;

    if (fabsf(robot.toolRotateDeg - servoRotateFilteredDeg) > 0.05f)
        servoRotateFilteredDeg += (robot.toolRotateDeg - servoRotateFilteredDeg) * SERVO_ROTATE_FILTER_ALPHA;
    else
        servoRotateFilteredDeg = robot.toolRotateDeg;

    // Схват: обмеження швидкості зміни кута -> серва рухається плавно, а не стрибком
    {
        const float maxStep = GRIP_OUTPUT_SLEW_DEG_S * servoDtS;
        const float err = robot.gripperDeg - servoGripFilteredDeg;
        servoGripFilteredDeg += clampFloat(err, -maxStep, maxStep);
    }

    static int lastRot = -1;
    const int pr = angleToPulseUs(servoRotateFilteredDeg, SERVO_ROTATE_MIN_US, SERVO_ROTATE_MAX_US);
    if (pr != lastRot)
    {
        servoRotate.writeMicroseconds(pr);
        lastRot = pr;
    }

    static int lastGrip = -1;
    const int pg = angleToPulseUs(servoGripFilteredDeg, SERVO_GRIP_MIN_US, SERVO_GRIP_MAX_US);
    if (pg != lastGrip)
    {
        servoGrip.writeMicroseconds(pg);
        lastGrip = pg;
    }
}

void initServos()
{
    servoRotate.setPeriodHertz(50);
    servoGrip.setPeriodHertz(50);
    servoRotate.attach(PIN_SERVO_ROTATE, SERVO_ROTATE_MIN_US, SERVO_ROTATE_MAX_US);
    servoGrip.attach(PIN_SERVO_GRIP, SERVO_GRIP_MIN_US, SERVO_GRIP_MAX_US);

    servoRotateFilteredDeg = robot.toolRotateDeg;
    servoGripFilteredDeg = robot.gripperDeg;
    servoRotate.writeMicroseconds(angleToPulseUs(servoRotateFilteredDeg, SERVO_ROTATE_MIN_US, SERVO_ROTATE_MAX_US));
    servoGrip.writeMicroseconds(angleToPulseUs(servoGripFilteredDeg, SERVO_GRIP_MIN_US, SERVO_GRIP_MAX_US));
}
