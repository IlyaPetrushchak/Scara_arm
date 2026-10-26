#include "auto_mode.h"
#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "utils.h"
#include "motion.h"
#include "program.h"

bool autoMotionStarted = false;
uint32_t autoDwellStartedMs = 0;

void setAutoFault(const char *reason)
{
    emergencyStopMotion();
    robot.autoState = AutoState::FAULT;
    autoMotionStarted = false;
    digitalWrite(PIN_LED_ERROR, HIGH);
    if (reason) Serial.println(reason);
}

void pauseAuto(const char *reason)
{
    if (robot.operatingMode != OperatingMode::AUTO) return;
    if (robot.autoState == AutoState::IDLE || robot.autoState == AutoState::FINISHED ||
        robot.autoState == AutoState::FAULT)
        return;

    emergencyStopMotion();
    robot.autoState = AutoState::PAUSED;
    autoMotionStarted = false;
    if (reason) Serial.println(reason);
}

void startAuto()
{
    if (robot.estopLatched || robot.operatingMode != OperatingMode::AUTO) return;
    if (programPointCount == 0)
    {
        Serial.println("AUTO: немає точок");
        return;
    }
    currentProgramPoint = 0;
    autoMotionStarted = false;
    robot.autoState = AutoState::MOVE;
    digitalWrite(PIN_LED_ERROR, LOW);
    Serial.printf("AUTO СТАРТ: %u точок\n", programPointCount);
}

void toggleAutoRunPause()
{
    if (robot.operatingMode != OperatingMode::AUTO || robot.estopLatched) return;

    switch (robot.autoState)
    {
    case AutoState::IDLE:
    case AutoState::FINISHED:
        startAuto();
        break;
    case AutoState::MOVE:
    case AutoState::DWELL:
        pauseAuto("AUTO ПАУЗА");
        break;
    case AutoState::PAUSED:
        robot.autoState = AutoState::MOVE;
        autoMotionStarted = false;
        Serial.printf("AUTO ПРОДОВЖЕННЯ: P%02u\n", currentProgramPoint + 1);
        break;
    case AutoState::FAULT:
        Serial.println("AUTO: FAULT — потрібен перезапуск");
        break;
    }
}

void startCurrentProgramPoint()
{
    if (currentProgramPoint >= programPointCount)
    {
        setAutoFault("AUTO: невірний індекс точки");
        return;
    }
    const ProgramPoint &p = programPoints[currentProgramPoint];
    robot.toolRotateDeg = clampFloat(p.toolRotateDeg, TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
    robot.gripperDeg = clampFloat(p.gripperDeg, GRIP_MIN_DEG, GRIP_MAX_DEG);
    commandAutoJointPosition(p.joints);
    autoMotionStarted = true;
    Serial.printf("AUTO -> P%02u/%02u\n", currentProgramPoint + 1, programPointCount);
}

void processAuto()
{
    if (robot.operatingMode != OperatingMode::AUTO || robot.estopLatched) return;

    switch (robot.autoState)
    {
    case AutoState::IDLE:
    case AutoState::PAUSED:
    case AutoState::FINISHED:
    case AutoState::FAULT:
        return;

    case AutoState::MOVE:
        if (!autoMotionStarted)
        {
            startCurrentProgramPoint();
            if (robot.autoState == AutoState::FAULT) return;
        }
        if (autoMotionStarted && motionFinished())
        {
            autoMotionStarted = false;
            autoDwellStartedMs = millis();
            robot.autoState = AutoState::DWELL;
        }
        break;

    case AutoState::DWELL:
    {
        if (millis() - autoDwellStartedMs < programPoints[currentProgramPoint].dwellMs) return;

        currentProgramPoint++;
        if (currentProgramPoint >= programPointCount)
        {
            if (AUTO_LOOP_PROGRAM)
            {
                currentProgramPoint = 0;
                Serial.println("AUTO: наступний цикл");
            }
            else
            {
                robot.autoState = AutoState::FINISHED;
                Serial.println("AUTO: програму завершено");
                return;
            }
        }
        robot.autoState = AutoState::MOVE;
        autoMotionStarted = false;
        break;
    }
    }
}
