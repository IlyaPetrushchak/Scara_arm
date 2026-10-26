#include "panel.h"
#include <Arduino.h>
#include "config.h"
#include "state.h"
#include "motion.h"
#include "column_cycle.h"
#include "program.h"
#include "auto_mode.h"
#include "gamepad.h"

uint32_t lastModeButtonEdgeMs = 0;
uint32_t lastTeachButtonEdgeMs = 0;
uint32_t teachPressStartedMs = 0;
bool previousModeButton = LOW;
bool previousTeachButton = LOW;

void initPanel()
{
    pinMode(PIN_LED_BT, OUTPUT);
    pinMode(PIN_LED_ERROR, OUTPUT);
    pinMode(PIN_LED_STATUS, OUTPUT);
    pinMode(PIN_BUTTON_TEACH, INPUT_PULLDOWN);
    pinMode(PIN_BUTTON_MODE, INPUT_PULLDOWN);
    pinMode(PIN_ESTOP, INPUT); // GPIO39 без внутрішнього підтягування

    previousTeachButton = digitalRead(PIN_BUTTON_TEACH);
    previousModeButton = digitalRead(PIN_BUTTON_MODE);

    digitalWrite(PIN_LED_BT, LOW);
    digitalWrite(PIN_LED_ERROR, LOW);
    digitalWrite(PIN_LED_STATUS, LOW);
}

void updateEmergencyStop()
{
    const bool active = digitalRead(PIN_ESTOP) == HIGH;
    if (active && !robot.estopLatched)
    {
        robot.estopLatched = true;
        robot.gamepadArmed = false;
        jogContinuousActive = false;
        emergencyStopMotion();
        robot.autoState = AutoState::FAULT;
        digitalWrite(PIN_LED_ERROR, HIGH);
        digitalWrite(PIN_LED_STATUS, LOW);
        Serial.println("E-STOP АКТИВНИЙ — потрібен перезапуск");
    }
}

void toggleControlMode()
{
    if (robot.estopLatched || !robot.gamepadArmed || robot.operatingMode != OperatingMode::MANUAL) return;
    robot.controlMode = (robot.controlMode == ControlMode::JOINT) ? ControlMode::TOOL : ControlMode::JOINT;
    Serial.println(robot.controlMode == ControlMode::TOOL ? "MANUAL: TOOL" : "MANUAL: JOINT");
}

void toggleOperatingMode()
{
    if (robot.estopLatched) return;
    if (axesRunning())
    {
        Serial.println("MODE: зупини осі перед зміною режиму");
        return;
    }

    if (robot.operatingMode == OperatingMode::MANUAL)
    {
        stopColumnCycle();
        robot.operatingMode = OperatingMode::AUTO;
        robot.autoState = AutoState::IDLE;
        robot.gamepadArmed = false;
        previousPadStart = false;
        previousPadY = false;
        padHoming = false;
        Serial.printf("Режим: AUTO (%u точок)\n", programPointCount);
    }
    else
    {
        emergencyStopMotion();
        robot.operatingMode = OperatingMode::MANUAL;
        robot.autoState = AutoState::IDLE;
        autoMotionStarted = false;
        robot.gamepadArmed = false;
        gamepadNeutralSinceMs = 0;
        previousPadStart = false;
        previousPadY = false;
        padHoming = false;
        Serial.println("Режим: MANUAL");
    }
    digitalWrite(PIN_LED_STATUS, robot.operatingMode == OperatingMode::AUTO ? HIGH : LOW);
}

void printCurrentPosition()
{
    updateActualRobotPosition();
    Serial.println("----- СТАН -----");
    Serial.printf("A1 %.2f/%.2f  A2 %.2f/%.2f  Z %.2f/%.2f\n",
                  robot.actualJoints.arm1Deg, robot.joints.arm1Deg,
                  robot.actualJoints.arm2Deg, robot.joints.arm2Deg,
                  robot.actualJoints.zMm, robot.joints.zMm);
    Serial.printf("TCP %.1f / %.1f  ROT %.1f  GRIP %.1f  точок %u/%u\n",
                  robot.actualTcp.xMm, robot.actualTcp.yMm, robot.toolRotateDeg,
                  robot.gripperDeg, programPointCount, MAX_PROGRAM_POINTS);
    Serial.println("----------------");
}

// Кнопки панелі: HIGH = натиснуто. TEACH: коротко = точка, довго = очистити
void updatePanelButtons()
{
    const uint32_t now = millis();
    const bool modeBtn = digitalRead(PIN_BUTTON_MODE);
    const bool teachBtn = digitalRead(PIN_BUTTON_TEACH);

    if (previousModeButton == LOW && modeBtn == HIGH && now - lastModeButtonEdgeMs >= BUTTON_DEBOUNCE_MS)
    {
        lastModeButtonEdgeMs = now;
        toggleOperatingMode();
    }

    if (previousTeachButton == LOW && teachBtn == HIGH && now - lastTeachButtonEdgeMs >= BUTTON_DEBOUNCE_MS)
    {
        lastTeachButtonEdgeMs = now;
        teachPressStartedMs = now;
    }

    if (previousTeachButton == HIGH && teachBtn == LOW && now - lastTeachButtonEdgeMs >= BUTTON_DEBOUNCE_MS)
    {
        lastTeachButtonEdgeMs = now;
        if (teachPressStartedMs != 0)
        {
            if (now - teachPressStartedMs >= TEACH_LONG_PRESS_MS)
                clearProgram();
            else
                teachCurrentPoint();
        }
        teachPressStartedMs = 0;
    }

    previousModeButton = modeBtn;
    previousTeachButton = teachBtn;
}
