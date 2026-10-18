/*
 * SCARA Robot Controller  —  чиста повна версія
 * ------------------------------------------------------------
 * 3-осьовий SCARA: геймпад (Bluepad32) + веб-панель по Wi-Fi SoftAP,
 * TEACH-точки в NVS, автоматичне відтворення, 3 колонки.
 *
 * Геймпад: кнопка Y = повернення в HOME (тільки MANUAL).
 *
 * HOME ручний: перед подачею живлення / ресетом постав руку в HOME
 * (значення нижче).
 *
 * Якщо Wi-Fi чомусь нестабільний — збери з  -DENABLE_GAMEPAD=0
 * (у platformio.ini: build_flags), тоді Bluetooth повністю вимкнений.
 */

#ifndef ENABLE_GAMEPAD
#define ENABLE_GAMEPAD 1
#endif

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include <esp_system.h>
#include <FastAccelStepper.h>
#include <ESP32Servo.h>
#if ENABLE_GAMEPAD
#include <Bluepad32.h>
#endif
#include "web.h"

// ============================================================
// ПІНИ
// ============================================================
constexpr uint8_t PIN_LED_BT = 4;
constexpr uint8_t PIN_LED_ERROR = 16;
constexpr uint8_t PIN_LED_STATUS = 17;

constexpr uint8_t PIN_STEP_ARM1 = 13;
constexpr uint8_t PIN_DIR_ARM1 = 26;
constexpr uint8_t PIN_STEP_ARM2 = 23;
constexpr uint8_t PIN_DIR_ARM2 = 14;
constexpr uint8_t PIN_STEP_Z = 18;
constexpr uint8_t PIN_DIR_Z = 19;

constexpr uint8_t PIN_SERVO_ROTATE = 27;
constexpr uint8_t PIN_SERVO_GRIP = 32;

constexpr uint8_t PIN_BUTTON_TEACH = 25;
constexpr uint8_t PIN_BUTTON_MODE = 33;
constexpr uint8_t PIN_ESTOP = 39;

// ============================================================
// МЕХАНІКА
// ============================================================
constexpr float ARM_LENGTH_1_MM = 91.61f;
constexpr float ARM_LENGTH_2_MM = 97.528f;

constexpr float MOTOR_STEPS_REV = 200.0f;
constexpr float MICROSTEPS = 16.0f;

constexpr float GEAR_ARM1 = 4.5f;
constexpr float GEAR_ARM2 = 7.280f;
constexpr float Z_SCREW_LEAD_MM = 2.0f;
constexpr float GEAR_Z = 1.0f;

// Знак осі: розворот напрямку без перепаювання
constexpr int8_t SIGN_ARM1 = 1;
constexpr int8_t SIGN_ARM2 = 1;
constexpr int8_t SIGN_Z = -1;

constexpr float STEPS_PER_DEG_ARM1 = MOTOR_STEPS_REV * MICROSTEPS * GEAR_ARM1 / 360.0f;
constexpr float STEPS_PER_DEG_ARM2 = MOTOR_STEPS_REV * MICROSTEPS * GEAR_ARM2 / 360.0f;
constexpr float STEPS_PER_MM_Z = MOTOR_STEPS_REV * MICROSTEPS * GEAR_Z / Z_SCREW_LEAD_MM;

// ============================================================
// HOME (ручний) і ПРОГРАМНІ ЛІМІТИ
// ============================================================
constexpr float HOME_ARM1_DEG = 115.0f;
constexpr float HOME_ARM2_DEG = -147.0f;
constexpr float HOME_Z_MM = 0.0f;
constexpr float HOME_TOOL_ROTATE_DEG = 90.0f;

constexpr float ARM1_MIN_DEG = -115.0f;
constexpr float ARM1_MAX_DEG = 115.0f;
// ВИПРАВЛЕНО: мінімум A2 тепер НЕ вищий за HOME (-147), інакше при першій
// команді рука сама смикалась на -138.
constexpr float ARM2_MIN_DEG = -150.0f;
constexpr float ARM2_MAX_DEG = 138.0f;
constexpr float Z_MIN_MM = 0.0f;
constexpr float Z_MAX_MM = 300.0f;
constexpr float TOOL_ROTATE_MIN_DEG = 0.0f;
constexpr float TOOL_ROTATE_MAX_DEG = 180.0f;

// ---- СХВАТ: РЕАЛЬНІ кути серви (0..180°) ----
// !!! ПІДІБРАТИ ПІД МЕХАНІКУ !!!
// GRIP_MIN_DEG = повністю відкрито (безпечна межа, не виходить із пазів)
// GRIP_MAX_DEG = повністю закрито.
// Зменшуй/збільшуй по 10°, поки схват не стане рівно в пазах.
constexpr float GRIP_MIN_DEG = 60.0f;
constexpr float GRIP_MAX_DEG = 130.0f;
constexpr float HOME_GRIP_DEG = GRIP_MIN_DEG;

constexpr float MIN_ARM_RADIUS_MM = 45.0f;
constexpr float MAX_ARM_RADIUS_MM = 185.0f;

// ============================================================
// ШВИДКОСТІ ТА ПРИСКОРЕННЯ
// ============================================================
constexpr float ARM1_JOG_SPEED_DEG_S = 45.0f;
constexpr float ARM2_JOG_SPEED_DEG_S = 45.0f;
constexpr float Z_JOG_SPEED_MM_S = 10.0f; // було 3.5
constexpr float TOOL_XY_SPEED_MM_S = 40.0f;

constexpr float AUTO_ARM1_SPEED_DEG_S = 40.0f;
constexpr float AUTO_ARM2_SPEED_DEG_S = 40.0f;
constexpr float AUTO_Z_SPEED_MM_S = 10.0f; // було 3.5

// Швидкість повернення в HOME (кнопка Y). Обмежена ARMx_MAX_STEP_HZ / Z_MAX_STEP_HZ.
constexpr float HOME_ARM1_SPEED_DEG_S = 90.0f; // 90 * 40 кроків/° = 3600 Гц
constexpr float HOME_ARM2_SPEED_DEG_S = 80.0f; // 80 * 64.7 кроків/° = ~5200 Гц
constexpr float HOME_Z_SPEED_MM_S = 12.0f;     // 12 * 1600 кроків/мм = 19200 Гц

constexpr float TOOL_ROTATE_SPEED_DEG_S = 45.0f;
constexpr float GRIP_SPEED_DEG_S = 300.0f; // було 140: менше "мертвого ходу" при відкритті

constexpr int SERVO_ROTATE_MIN_US = 500;
constexpr int SERVO_ROTATE_MAX_US = 2500;
constexpr int SERVO_GRIP_MIN_US = 500;
constexpr int SERVO_GRIP_MAX_US = 2500;
constexpr float SERVO_ROTATE_FILTER_ALPHA = 0.45f;

constexpr uint32_t ARM1_MAX_STEP_HZ = 12000;
constexpr uint32_t ARM2_MAX_STEP_HZ = 6000;
constexpr uint32_t Z_MAX_STEP_HZ = 20000; // 20000 / 1600 = 12.5 мм/с

constexpr uint32_t ARM1_ACCEL = 14000;
constexpr uint32_t ARM2_ACCEL = 8000;
constexpr uint32_t Z_ACCEL = 20000; // було 6000: швидший розгін/гальмування

// ============================================================
// 3 КОЛОНКИ
// ============================================================
constexpr float COLUMN_X_MM[3] = {-75.0f, 0.0f, 75.0f};
constexpr float COLUMN_Y_MM[3] = {110.0f, 135.0f, 110.0f};
constexpr float COLUMN_Z_TOP_MM = 15.0f;
constexpr float COLUMN_Z_BOTTOM_MM = 120.0f;
constexpr uint32_t COLUMN_DWELL_MS = 350;

// ============================================================
// ЧАС І НАЛАШТУВАННЯ
// ============================================================
constexpr uint32_t CONTROL_PERIOD_MS = 10;
constexpr uint32_t SERVO_PERIOD_MS = 20;
constexpr uint32_t GAMEPAD_TIMEOUT_MS = 10000;
constexpr uint32_t GAMEPAD_ARM_NEUTRAL_MS = 300;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint32_t TEACH_LONG_PRESS_MS = 1800;
constexpr uint32_t DEFAULT_POINT_DWELL_MS = 400;
constexpr uint32_t JOG_WATCHDOG_MS = 1800; // веб шле джог кожні 0.6 с
constexpr uint32_t DIAG_PERIOD_MS = 5000;
constexpr int16_t JOYSTICK_DEADZONE = 90;
constexpr float MAX_CONTROL_DT_S = 0.05f;
constexpr uint8_t MAX_PROGRAM_POINTS = 20;
constexpr bool AUTO_LOOP_PROGRAM = true;

// ---- Wi-Fi ----
static const char *AP_SSID = "RoboArm-WiFi";
static const char *AP_PASS = "12345678";
constexpr uint8_t AP_CHANNEL = 1;
constexpr uint8_t AP_MAX_CLIENTS = 3;

// ============================================================
// ТИПИ
// ============================================================
enum class ControlMode : uint8_t { JOINT, TOOL };
enum class OperatingMode : uint8_t { MANUAL, AUTO };
enum class AutoState : uint8_t { IDLE, MOVE, DWELL, PAUSED, FINISHED, FAULT };

struct JointPosition { float arm1Deg; float arm2Deg; float zMm; };
struct CartesianPosition { float xMm; float yMm; float zMm; };

struct ProgramPoint
{
    JointPosition joints;
    float toolRotateDeg;
    float gripperDeg;
    uint32_t dwellMs;
};

struct RobotState
{
    JointPosition joints;        // команда (ціль)
    JointPosition actualJoints;  // факт зі лічильників кроків
    CartesianPosition tcp;
    CartesianPosition actualTcp;
    float toolRotateDeg;
    float gripperDeg;
    ControlMode controlMode;
    OperatingMode operatingMode;
    AutoState autoState;
    bool estopLatched;
    bool controllerConnected;
    bool gamepadArmed;
};

// ============================================================
// ГЛОБАЛЬНИЙ СТАН
// ============================================================
FastAccelStepperEngine stepperEngine;
FastAccelStepper *stepperArm1 = nullptr;
FastAccelStepper *stepperArm2 = nullptr;
FastAccelStepper *stepperZ = nullptr;

Servo servoRotate;
Servo servoGrip;
Preferences preferences;
RobotState robot;
WebServer server(80);

#if ENABLE_GAMEPAD
ControllerPtr controllers[BP32_MAX_GAMEPADS];
#endif

ProgramPoint programPoints[MAX_PROGRAM_POINTS];
uint8_t programPointCount = 0;
uint8_t currentProgramPoint = 0;
bool autoMotionStarted = false;
uint32_t autoDwellStartedMs = 0;

uint32_t lastControlMs = 0;
uint32_t lastServoUpdateMs = 0;
uint32_t lastGamepadPacketMs = 0;
uint32_t gamepadNeutralSinceMs = 0;
uint32_t lastModeButtonEdgeMs = 0;
uint32_t lastTeachButtonEdgeMs = 0;
uint32_t teachPressStartedMs = 0;

float servoRotateFilteredDeg = HOME_TOOL_ROTATE_DEG;
float servoGripFilteredDeg = HOME_GRIP_DEG;

bool previousModeButton = LOW;
bool previousTeachButton = LOW;
bool previousPadStart = false;
bool previousPadY = false; // Y на геймпаді (повернення в HOME)
bool padHoming = false;    // іде повернення в HOME з геймпада
uint32_t jogWatchdogMs = 0;
bool jogContinuousActive = false;

// ============================================================
// ДОПОМІЖНІ ФУНКЦІЇ
// ============================================================
float clampFloat(float v, float lo, float hi)
{
    if (v < lo) return lo;
    if (v > hi) return hi;
    return v;
}

uint32_t clampStepHz(float hz, uint32_t maxHz)
{
    if (hz < 1.0f) return 1;
    if (hz > static_cast<float>(maxHz)) return maxHz;
    return static_cast<uint32_t>(lroundf(hz));
}

float degToRad(float d) { return d * PI / 180.0f; }
float radToDeg(float r) { return r * 180.0f / PI; }

// Нормалізація осі: плавна мертва зона + крива x^1.5
float normalizeJoystick(int32_t value)
{
    const int32_t absVal = abs(value);
    if (absVal <= JOYSTICK_DEADZONE) return 0.0f;
    constexpr float maxAxis = 512.0f;
    const float n = static_cast<float>(absVal - JOYSTICK_DEADZONE) / (maxAxis - JOYSTICK_DEADZONE);
    const float curved = powf(clampFloat(n, 0.0f, 1.0f), 1.5f);
    return (value < 0) ? -curved : curved;
}

int32_t arm1DegreesToSteps(float a) { return lroundf(a * STEPS_PER_DEG_ARM1 * SIGN_ARM1); }
int32_t arm2DegreesToSteps(float a) { return lroundf(a * STEPS_PER_DEG_ARM2 * SIGN_ARM2); }
int32_t zMillimetersToSteps(float z) { return lroundf(z * STEPS_PER_MM_Z * SIGN_Z); }
float arm1StepsToDegrees(int32_t s) { return static_cast<float>(s) / (STEPS_PER_DEG_ARM1 * SIGN_ARM1); }
float arm2StepsToDegrees(int32_t s) { return static_cast<float>(s) / (STEPS_PER_DEG_ARM2 * SIGN_ARM2); }
float zStepsToMillimeters(int32_t s) { return static_cast<float>(s) / (STEPS_PER_MM_Z * SIGN_Z); }

// ============================================================
// КІНЕМАТИКА
// ============================================================
CartesianPosition forwardKinematics(const JointPosition &j)
{
    const float t1 = degToRad(j.arm1Deg);
    const float t2 = degToRad(j.arm2Deg);
    CartesianPosition r;
    r.xMm = ARM_LENGTH_1_MM * cosf(t1) + ARM_LENGTH_2_MM * cosf(t1 + t2);
    r.yMm = ARM_LENGTH_1_MM * sinf(t1) + ARM_LENGTH_2_MM * sinf(t1 + t2);
    r.zMm = j.zMm;
    return r;
}

// elbowUp=true -> A2 >= 0; false -> A2 < 0
bool inverseKinematics(float xMm, float yMm, bool elbowUp, JointPosition &out)
{
    const float r2 = xMm * xMm + yMm * yMm;
    float c = (r2 - ARM_LENGTH_1_MM * ARM_LENGTH_1_MM - ARM_LENGTH_2_MM * ARM_LENGTH_2_MM) /
              (2.0f * ARM_LENGTH_1_MM * ARM_LENGTH_2_MM);
    if (c < -1.0f || c > 1.0f) return false;

    float s = sqrtf(1.0f - c * c);
    if (!elbowUp) s = -s;

    const float t2 = atan2f(s, c);
    const float t1 = atan2f(yMm, xMm) - atan2f(ARM_LENGTH_2_MM * s, ARM_LENGTH_1_MM + ARM_LENGTH_2_MM * c);

    out.arm1Deg = radToDeg(t1);
    out.arm2Deg = radToDeg(t2);
    out.zMm = robot.joints.zMm;

    return !(out.arm1Deg < ARM1_MIN_DEG || out.arm1Deg > ARM1_MAX_DEG ||
             out.arm2Deg < ARM2_MIN_DEG || out.arm2Deg > ARM2_MAX_DEG);
}

// ============================================================
// ПОЗИЦІЯ І РУХ
// ============================================================
void updateActualRobotPosition()
{
    if (!stepperArm1 || !stepperArm2 || !stepperZ) return;
    robot.actualJoints.arm1Deg = arm1StepsToDegrees(stepperArm1->getCurrentPosition());
    robot.actualJoints.arm2Deg = arm2StepsToDegrees(stepperArm2->getCurrentPosition());
    robot.actualJoints.zMm = zStepsToMillimeters(stepperZ->getCurrentPosition());
    robot.actualTcp = forwardKinematics(robot.actualJoints);
}

bool axesRunning()
{
    if (!stepperArm1 || !stepperArm2 || !stepperZ) return false;
    return stepperArm1->isRunning() || stepperArm2->isRunning() || stepperZ->isRunning();
}

bool motionFinished() { return !axesRunning(); }

void syncCommandToActual()
{
    updateActualRobotPosition();
    robot.joints = robot.actualJoints;
    robot.tcp = robot.actualTcp;
}

JointPosition clampJointPosition(const JointPosition &t)
{
    JointPosition c = {
        clampFloat(t.arm1Deg, ARM1_MIN_DEG, ARM1_MAX_DEG),
        clampFloat(t.arm2Deg, ARM2_MIN_DEG, ARM2_MAX_DEG),
        clampFloat(t.zMm, Z_MIN_MM, Z_MAX_MM)};

    // Захист від складання руки в колону основи
    const CartesianPosition p = forwardKinematics(c);
    const float r = sqrtf(p.xMm * p.xMm + p.yMm * p.yMm);
    if (r < MIN_ARM_RADIUS_MM && fabsf(c.arm2Deg) > 130.0f)
        c.arm2Deg = (c.arm2Deg < 0.0f) ? -130.0f : 130.0f;
    return c;
}

void stopColumnCycle();

// Миттєва зупинка генераторів кроків
void emergencyStopMotion()
{
    padHoming = false; // будь-яка аварійна зупинка скасовує HOME з геймпада
    stopColumnCycle();
    if (stepperArm1) stepperArm1->forceStopAndNewPosition(stepperArm1->getCurrentPosition());
    if (stepperArm2) stepperArm2->forceStopAndNewPosition(stepperArm2->getCurrentPosition());
    if (stepperZ) stepperZ->forceStopAndNewPosition(stepperZ->getCurrentPosition());
    syncCommandToActual();
}

// Ручний рух з геймпада: малі цілі, оновлюються щоцикл
void commandManualJointPosition(const JointPosition &target, float dt)
{
    if (robot.estopLatched || !robot.gamepadArmed || robot.operatingMode != OperatingMode::MANUAL)
        return;

    const JointPosition c = clampJointPosition(target);

    if (dt > 0.0f)
    {
        const float d1 = fabsf(c.arm1Deg - robot.joints.arm1Deg);
        const float d2 = fabsf(c.arm2Deg - robot.joints.arm2Deg);
        const float dz = fabsf(c.zMm - robot.joints.zMm);

        const uint32_t m1 = clampStepHz(ARM1_JOG_SPEED_DEG_S * STEPS_PER_DEG_ARM1, ARM1_MAX_STEP_HZ);
        const uint32_t m2 = clampStepHz(ARM2_JOG_SPEED_DEG_S * STEPS_PER_DEG_ARM2, ARM2_MAX_STEP_HZ);
        const uint32_t mz = clampStepHz(Z_JOG_SPEED_MM_S * STEPS_PER_MM_Z, Z_MAX_STEP_HZ);

        if (d1 > 0.0001f) stepperArm1->setSpeedInHz(clampStepHz(d1 / dt * STEPS_PER_DEG_ARM1, m1));
        if (d2 > 0.0001f) stepperArm2->setSpeedInHz(clampStepHz(d2 / dt * STEPS_PER_DEG_ARM2, m2));
        if (dz > 0.0001f) stepperZ->setSpeedInHz(clampStepHz(dz / dt * STEPS_PER_MM_Z, mz));
    }

    stepperArm1->moveTo(arm1DegreesToSteps(c.arm1Deg));
    stepperArm2->moveTo(arm2DegreesToSteps(c.arm2Deg));
    stepperZ->moveTo(zMillimetersToSteps(c.zMm));

    robot.joints = c;
    robot.tcp = forwardKinematics(robot.joints);
}

// Координований PTP: усі осі приходять одночасно
void commandCoordinatedJointPosition(const JointPosition &target,
                                     float maxSpeed1, float maxSpeed2, float maxSpeedZ)
{
    if (robot.estopLatched) return;

    updateActualRobotPosition();
    const JointPosition c = clampJointPosition(target);

    const float d1 = fabsf(c.arm1Deg - robot.actualJoints.arm1Deg);
    const float d2 = fabsf(c.arm2Deg - robot.actualJoints.arm2Deg);
    const float dz = fabsf(c.zMm - robot.actualJoints.zMm);

    const float moveTime = fmaxf(0.04f, fmaxf(d1 / maxSpeed1, fmaxf(d2 / maxSpeed2, dz / maxSpeedZ)));

    const uint32_t l1 = clampStepHz(maxSpeed1 * STEPS_PER_DEG_ARM1, ARM1_MAX_STEP_HZ);
    const uint32_t l2 = clampStepHz(maxSpeed2 * STEPS_PER_DEG_ARM2, ARM2_MAX_STEP_HZ);
    const uint32_t lz = clampStepHz(maxSpeedZ * STEPS_PER_MM_Z, Z_MAX_STEP_HZ);

    if (d1 > 0.001f) stepperArm1->setSpeedInHz(clampStepHz(d1 / moveTime * STEPS_PER_DEG_ARM1, l1));
    if (d2 > 0.001f) stepperArm2->setSpeedInHz(clampStepHz(d2 / moveTime * STEPS_PER_DEG_ARM2, l2));
    if (dz > 0.001f) stepperZ->setSpeedInHz(clampStepHz(dz / moveTime * STEPS_PER_MM_Z, lz));

    const int32_t s1 = arm1DegreesToSteps(c.arm1Deg);
    const int32_t s2 = arm2DegreesToSteps(c.arm2Deg);
    const int32_t sz = zMillimetersToSteps(c.zMm);

    if (stepperArm1->targetPos() != s1) stepperArm1->moveTo(s1);
    if (stepperArm2->targetPos() != s2) stepperArm2->moveTo(s2);
    if (stepperZ->targetPos() != sz) stepperZ->moveTo(sz);

    robot.joints = c;
    robot.tcp = forwardKinematics(robot.joints);
}

void commandAutoJointPosition(const JointPosition &t)
{
    if (robot.estopLatched || robot.operatingMode != OperatingMode::AUTO) return;
    commandCoordinatedJointPosition(t, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
}

void commandWebJointPosition(const JointPosition &t)
{
    commandCoordinatedJointPosition(t, ARM1_JOG_SPEED_DEG_S, ARM2_JOG_SPEED_DEG_S, Z_JOG_SPEED_MM_S);
}

// ============================================================
// ЦИКЛ 3 КОЛОНОК
// ============================================================
enum class ColumnStep : uint8_t { IDLE, GOTO_TOP, DESCEND, DWELL, ASCEND, NEXT_COL };

bool columnCycleActive = false;
ColumnStep columnStep = ColumnStep::IDLE;
uint8_t currentColumnId = 0;  // 1..3
uint8_t targetColumnMode = 0; // 0 = усі, 1..3 = одна
uint32_t columnDwellTimer = 0;

void startColumnCycle(uint8_t mode)
{
    if (robot.estopLatched) return;
    targetColumnMode = mode;
    currentColumnId = (mode >= 1 && mode <= 3) ? mode : 1;
    columnStep = ColumnStep::GOTO_TOP;
    columnCycleActive = true;
    columnDwellTimer = 0;
    Serial.printf("КОЛОНКИ: старт, режим %u\n", targetColumnMode);
}

void stopColumnCycle()
{
    if (!columnCycleActive) return;
    columnCycleActive = false;
    columnStep = ColumnStep::IDLE;
    columnDwellTimer = 0;
    Serial.println("КОЛОНКИ: зупинено");
}

void processColumnCycle()
{
    if (!columnCycleActive || robot.estopLatched) return;
    if (!motionFinished()) return;

    const uint8_t idx = currentColumnId - 1;
    JointPosition col;
    if (idx > 2 || !inverseKinematics(COLUMN_X_MM[idx], COLUMN_Y_MM[idx], true, col))
    {
        Serial.printf("КОЛОНКИ: IK недосяжна, колонка %u\n", currentColumnId);
        stopColumnCycle();
        return;
    }

    const uint32_t now = millis();

    switch (columnStep)
    {
    case ColumnStep::GOTO_TOP:
        col.zMm = COLUMN_Z_TOP_MM;
        commandCoordinatedJointPosition(col, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
        columnStep = ColumnStep::DESCEND;
        break;

    case ColumnStep::DESCEND:
        col.zMm = COLUMN_Z_BOTTOM_MM;
        commandCoordinatedJointPosition(col, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
        columnStep = ColumnStep::DWELL;
        columnDwellTimer = 0;
        break;

    case ColumnStep::DWELL:
        if (columnDwellTimer == 0)
            columnDwellTimer = now;
        else if (now - columnDwellTimer >= COLUMN_DWELL_MS)
        {
            columnDwellTimer = 0;
            columnStep = ColumnStep::ASCEND;
        }
        break;

    case ColumnStep::ASCEND:
        col.zMm = COLUMN_Z_TOP_MM;
        commandCoordinatedJointPosition(col, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
        columnStep = ColumnStep::NEXT_COL;
        break;

    case ColumnStep::NEXT_COL:
        if (targetColumnMode >= 1 && targetColumnMode <= 3)
            stopColumnCycle();
        else if (currentColumnId < 3)
        {
            currentColumnId++;
            columnStep = ColumnStep::GOTO_TOP;
        }
        else
        {
            Serial.println("КОЛОНКИ: усі 3 пройдено");
            stopColumnCycle();
        }
        break;

    default:
        stopColumnCycle();
        break;
    }
}

// ============================================================
// TEACH-ПРОГРАМА В NVS
// ============================================================
void saveProgram()
{
    preferences.putUChar("count", programPointCount);
    if (programPointCount > 0)
        preferences.putBytes("points", programPoints, sizeof(ProgramPoint) * programPointCount);
    else if (preferences.isKey("points"))
        preferences.remove("points");
}

void loadProgram()
{
    programPointCount = preferences.getUChar("count", 0);
    if (programPointCount > MAX_PROGRAM_POINTS)
    {
        programPointCount = 0;
        saveProgram();
        return;
    }
    if (programPointCount > 0)
    {
        const size_t expected = sizeof(ProgramPoint) * programPointCount;
        if (preferences.getBytesLength("points") != expected ||
            preferences.getBytes("points", programPoints, expected) != expected)
        {
            programPointCount = 0;
            saveProgram();
        }
    }
}

void clearProgram()
{
    if (robot.operatingMode != OperatingMode::MANUAL || axesRunning()) return;
    programPointCount = 0;
    currentProgramPoint = 0;
    saveProgram();
    Serial.println("TEACH: програму очищено");
}

void teachCurrentPoint()
{
    if (robot.estopLatched || robot.operatingMode != OperatingMode::MANUAL) return;
    if (axesRunning())
    {
        Serial.println("TEACH: спочатку зупини осі");
        return;
    }
    if (programPointCount >= MAX_PROGRAM_POINTS)
    {
        Serial.println("TEACH: програма заповнена");
        return;
    }

    updateActualRobotPosition();
    ProgramPoint &p = programPoints[programPointCount];
    p.joints = robot.actualJoints;
    p.toolRotateDeg = robot.toolRotateDeg;
    p.gripperDeg = robot.gripperDeg;
    p.dwellMs = DEFAULT_POINT_DWELL_MS;

    programPointCount++;
    saveProgram();

    Serial.printf("TEACH: P%02u  A1=%.2f A2=%.2f Z=%.2f ROT=%.1f GRIP=%.1f\n",
                  programPointCount, p.joints.arm1Deg, p.joints.arm2Deg, p.joints.zMm,
                  p.toolRotateDeg, p.gripperDeg);
}

// ============================================================
// АВТОМАТИЧНИЙ РЕЖИМ
// ============================================================
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

// ============================================================
// E-STOP
// ============================================================
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

// ============================================================
// РЕЖИМИ І ПАНЕЛЬ
// ============================================================
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

// ============================================================
// СЕРВИ НАРЯДУ
// ============================================================
// Кути серв — РЕАЛЬНІ (0..180°) -> імпульс
int angleToPulseUs(float angleDeg, int minUs, int maxUs)
{
    const float a = clampFloat(angleDeg, 0.0f, 180.0f);
    return lroundf(minUs + (a / 180.0f) * (maxUs - minUs));
}

void updateServoTargets(float rotateCmd, float gripCmd, float dt)
{
    robot.toolRotateDeg += rotateCmd * TOOL_ROTATE_SPEED_DEG_S * dt;
    robot.gripperDeg += gripCmd * GRIP_SPEED_DEG_S * dt;
    robot.toolRotateDeg = clampFloat(robot.toolRotateDeg, TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
    robot.gripperDeg = clampFloat(robot.gripperDeg, GRIP_MIN_DEG, GRIP_MAX_DEG);
}

void refreshServos()
{
    if (robot.estopLatched) return;

    const uint32_t now = millis();
    if (now - lastServoUpdateMs < SERVO_PERIOD_MS) return;
    lastServoUpdateMs = now;

    if (fabsf(robot.toolRotateDeg - servoRotateFilteredDeg) > 0.05f)
        servoRotateFilteredDeg += (robot.toolRotateDeg - servoRotateFilteredDeg) * SERVO_ROTATE_FILTER_ALPHA;
    else
        servoRotateFilteredDeg = robot.toolRotateDeg;

    servoGripFilteredDeg = robot.gripperDeg;

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

// ============================================================
// ГЕЙМПАД (Bluepad32)
// ============================================================
#if ENABLE_GAMEPAD

ControllerPtr getActiveController()
{
    for (ControllerPtr c : controllers)
        if (c != nullptr && c->isConnected() && c->isGamepad())
            return c;
    return nullptr;
}

void updateControllerConnectedFlag()
{
    robot.controllerConnected = getActiveController() != nullptr;
    digitalWrite(PIN_LED_BT, robot.controllerConnected ? HIGH : LOW);
}

void onConnectedController(ControllerPtr controller)
{
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        if (controllers[i] == nullptr)
        {
            controllers[i] = controller;
            lastGamepadPacketMs = millis();
            robot.gamepadArmed = false;
            gamepadNeutralSinceMs = 0;
            updateControllerConnectedFlag();
            Serial.printf("Геймпад підключено, слот %d\n", i);
            return;
        }
    }
}

void onDisconnectedController(ControllerPtr controller)
{
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
        if (controllers[i] == controller) controllers[i] = nullptr;

    if (robot.operatingMode == OperatingMode::AUTO)
        pauseAuto("Геймпад відключено — AUTO ПАУЗА");
    else
    {
        if (robot.gamepadArmed) emergencyStopMotion();
        robot.gamepadArmed = false;
        Serial.println("Геймпад відключено — MANUAL зупинено");
    }
    updateControllerConnectedFlag();
}

bool gamepadControlsNeutral(float lx, float ly, float rx, float ry, ControllerPtr c)
{
    return lx == 0.0f && ly == 0.0f && rx == 0.0f && ry == 0.0f &&
           !c->a() && !c->b() && !c->y() && !c->miscStart();
}

// Після підключення pad має 0.3 с постояти нейтральним
bool updateGamepadArming(float lx, float ly, float rx, float ry, ControllerPtr c)
{
    if (robot.operatingMode != OperatingMode::MANUAL) return false;
    if (robot.gamepadArmed) return true;

    const uint32_t now = millis();
    if (!gamepadControlsNeutral(lx, ly, rx, ry, c))
    {
        gamepadNeutralSinceMs = 0;
        return false;
    }
    if (gamepadNeutralSinceMs == 0)
    {
        gamepadNeutralSinceMs = now;
        return false;
    }
    if (now - gamepadNeutralSinceMs >= GAMEPAD_ARM_NEUTRAL_MS)
    {
        robot.gamepadArmed = true;
        previousPadStart = c->miscStart();
        previousPadY = c->y();
        digitalWrite(PIN_LED_ERROR, LOW);
        Serial.println("Геймпад ARMED");
        return true;
    }
    return false;
}

// Y на геймпаді: повернення в стартову позицію (HOME)
void goHomeFromGamepad()
{
    if (robot.estopLatched || robot.operatingMode != OperatingMode::MANUAL) return;

    stopColumnCycle();
    jogContinuousActive = false;

    const JointPosition home = {HOME_ARM1_DEG, HOME_ARM2_DEG, HOME_Z_MM};
    commandCoordinatedJointPosition(home, HOME_ARM1_SPEED_DEG_S, HOME_ARM2_SPEED_DEG_S, HOME_Z_SPEED_MM_S);
    robot.toolRotateDeg = HOME_TOOL_ROTATE_DEG;
    robot.gripperDeg = HOME_GRIP_DEG;

    padHoming = true;
    Serial.println("ГЕЙМПАД: Y — повернення в HOME");
}

void processJointMode(float ax, float ay, float az, float dt)
{
    JointPosition t = robot.joints;
    t.arm1Deg += ax * ARM1_JOG_SPEED_DEG_S * dt;
    t.arm2Deg += -ay * ARM2_JOG_SPEED_DEG_S * dt;
    t.zMm += az * Z_JOG_SPEED_MM_S * dt; // стік вгору (az<0) -> рука вгору (Z до 0)
    commandManualJointPosition(t, dt);
}

// TOOL: XY через IK. Гілка ліктя береться з ПОТОЧНОГО знака A2 — рука не перекидається.
void processToolMode(float ax, float ay, float az, float dt)
{
    CartesianPosition d = robot.tcp;
    d.xMm += ax * TOOL_XY_SPEED_MM_S * dt;
    d.yMm += -ay * TOOL_XY_SPEED_MM_S * dt;
    d.zMm = clampFloat(d.zMm + (az * Z_JOG_SPEED_MM_S * dt), Z_MIN_MM, Z_MAX_MM);

    const bool elbowUp = robot.joints.arm2Deg >= 0.0f;
    JointPosition j;
    if (!inverseKinematics(d.xMm, d.yMm, elbowUp, j))
    {
        digitalWrite(PIN_LED_ERROR, HIGH);
        return;
    }
    j.zMm = d.zMm;
    digitalWrite(PIN_LED_ERROR, LOW);
    commandManualJointPosition(j, dt);
}

void processGamepad(float dt)
{
    ControllerPtr c = getActiveController();

    if (c == nullptr)
    {
        if (robot.operatingMode == OperatingMode::AUTO &&
            (robot.autoState == AutoState::MOVE || robot.autoState == AutoState::DWELL))
            pauseAuto("Немає геймпада — AUTO ПАУЗА");
        return;
    }

    const uint32_t now = millis();
    const float lx = normalizeJoystick(c->axisX());
    const float ly = normalizeJoystick(c->axisY());
    const float rx = normalizeJoystick(c->axisRX());
    const float ry = normalizeJoystick(c->axisRY());
    const bool padStart = c->miscStart();

    if (robot.operatingMode == OperatingMode::AUTO)
    {
        if (padStart && !previousPadStart) toggleAutoRunPause();
        previousPadStart = padStart;
        return;
    }

    if (c->hasData()) lastGamepadPacketMs = now;

    // ВИПРАВЛЕНО: таймаут спрацьовує один раз, без спаму в Serial
    static bool timedOut = false;
    if (now - lastGamepadPacketMs > GAMEPAD_TIMEOUT_MS)
    {
        if (!timedOut)
        {
            timedOut = true;
            if (robot.gamepadArmed) emergencyStopMotion();
            robot.gamepadArmed = false;
            gamepadNeutralSinceMs = 0;
            Serial.println("Таймаут геймпада — MANUAL зупинено");
        }
        return;
    }
    timedOut = false;

    if (!updateGamepadArming(lx, ly, rx, ry, c))
    {
        previousPadStart = padStart;
        previousPadY = c->y();
        return;
    }

    if (padStart && !previousPadStart) toggleControlMode();
    previousPadStart = padStart;

    // Y: повернення в HOME
    const bool padY = c->y();
    if (padY && !previousPadY) goHomeFromGamepad();
    previousPadY = padY;

    if (padHoming && motionFinished()) padHoming = false;

    float grip = 0.0f;
    if (c->a()) grip = 1.0f;
    if (c->b()) grip = -1.0f;

    float rotCmd = rx;
    const float zCmd = ry;
    if (fabsf(ry) > 0.15f && fabsf(rx) < 0.30f) rotCmd = 0.0f; // Z без випадкового повороту

    // Під час повернення в HOME ручне керування серв не втручається в ціль
    if (!padHoming && (rotCmd != 0.0f || grip != 0.0f)) updateServoTargets(rotCmd, grip, dt);

    const bool stickMoved = fabsf(lx) > 0.05f || fabsf(ly) > 0.05f || fabsf(zCmd) > 0.05f;
    static bool wasStickActive = false;

    if (stickMoved)
    {
        // Рух стіком скасовує повернення в HOME (emergencyStopMotion скине padHoming
        // і синхронізує команду з фактом)
        if (padHoming) emergencyStopMotion();

        wasStickActive = true;
        jogContinuousActive = false; // геймпад важливіший за веб-джог
        if (robot.controlMode == ControlMode::JOINT)
            processJointMode(lx, ly, zCmd, dt);
        else
            processToolMode(lx, ly, zCmd, dt);
    }
    else if (wasStickActive)
    {
        wasStickActive = false;
        if (stepperArm1) stepperArm1->stopMove();
        if (stepperArm2) stepperArm2->stopMove();
        if (stepperZ) stepperZ->stopMove();
        syncCommandToActual();
    }
}

#else // ENABLE_GAMEPAD == 0

void updateControllerConnectedFlag() { robot.controllerConnected = false; }
void processGamepad(float) {}

#endif

// ============================================================
// КРОКОВИКИ
// ============================================================
bool initializeSteppers()
{
    stepperEngine.init(1);

    stepperArm1 = stepperEngine.stepperConnectToPin(PIN_STEP_ARM1);
    stepperArm2 = stepperEngine.stepperConnectToPin(PIN_STEP_ARM2);
    stepperZ = stepperEngine.stepperConnectToPin(PIN_STEP_Z);
    if (!stepperArm1 || !stepperArm2 || !stepperZ) return false;

    stepperArm1->setDirectionPin(PIN_DIR_ARM1, true, 200);
    stepperArm2->setDirectionPin(PIN_DIR_ARM2, true, 200);
    stepperZ->setDirectionPin(PIN_DIR_Z, true, 200);

    stepperArm1->setSpeedInHz(clampStepHz(ARM1_JOG_SPEED_DEG_S * STEPS_PER_DEG_ARM1, ARM1_MAX_STEP_HZ));
    stepperArm2->setSpeedInHz(clampStepHz(ARM2_JOG_SPEED_DEG_S * STEPS_PER_DEG_ARM2, ARM2_MAX_STEP_HZ));
    stepperZ->setSpeedInHz(clampStepHz(Z_JOG_SPEED_MM_S * STEPS_PER_MM_Z, Z_MAX_STEP_HZ));

    stepperArm1->setAcceleration(ARM1_ACCEL);
    stepperArm2->setAcceleration(ARM2_ACCEL);
    stepperZ->setAcceleration(Z_ACCEL);

    // Ручний homing: поточне положення = HOME
    stepperArm1->setCurrentPosition(arm1DegreesToSteps(HOME_ARM1_DEG));
    stepperArm2->setCurrentPosition(arm2DegreesToSteps(HOME_ARM2_DEG));
    stepperZ->setCurrentPosition(zMillimetersToSteps(HOME_Z_MM));
    return true;
}

// ============================================================
// ВЕБ-СЕРВЕР
// ============================================================
void sendJson(int code, const String &body)
{
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json; charset=utf-8", body);
}

// Ручні команди з вебу дозволені лише в MANUAL і без E-STOP
bool webGuard()
{
    if (robot.estopLatched)
    {
        sendJson(403, "{\"error\":\"E-STOP активний\"}");
        return false;
    }
    if (robot.operatingMode != OperatingMode::MANUAL)
    {
        sendJson(409, "{\"error\":\"Зараз режим AUTO — перейдіть у ручний\"}");
        return false;
    }
    return true;
}

void handleRoot()
{
    server.sendHeader("Cache-Control", "no-cache");
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

// Конфіг для веб-інтерфейсу: ліміти й геометрія завжди збігаються з прошивкою
void handleConfig()
{
    String j = "{";
    j += "\"a1\":[" + String(ARM1_MIN_DEG, 1) + "," + String(ARM1_MAX_DEG, 1) + "],";
    j += "\"a2\":[" + String(ARM2_MIN_DEG, 1) + "," + String(ARM2_MAX_DEG, 1) + "],";
    j += "\"z\":[" + String(Z_MIN_MM, 1) + "," + String(Z_MAX_MM, 1) + "],";
    j += "\"rot\":[" + String(TOOL_ROTATE_MIN_DEG, 1) + "," + String(TOOL_ROTATE_MAX_DEG, 1) + "],";
    j += "\"g\":[" + String(GRIP_MIN_DEG, 1) + "," + String(GRIP_MAX_DEG, 1) + "],";
    j += "\"l1\":" + String(ARM_LENGTH_1_MM, 3) + ",";
    j += "\"l2\":" + String(ARM_LENGTH_2_MM, 3) + ",";
    j += "\"rmin\":" + String(MIN_ARM_RADIUS_MM, 1) + ",";
    j += "\"rmax\":" + String(MAX_ARM_RADIUS_MM, 1) + ",";
    j += "\"pts\":" + String(MAX_PROGRAM_POINTS) + ",";
    j += "\"cols\":[";
    for (int i = 0; i < 3; i++)
    {
        if (i) j += ",";
        j += "[" + String(COLUMN_X_MM[i], 1) + "," + String(COLUMN_Y_MM[i], 1) + "]";
    }
    j += "]}";
    sendJson(200, j);
}

void handleState()
{
    updateActualRobotPosition();
    String j = "{";
    j += "\"a1\":" + String(robot.joints.arm1Deg, 2) + ",";
    j += "\"a2\":" + String(robot.joints.arm2Deg, 2) + ",";
    j += "\"z\":" + String(robot.joints.zMm, 2) + ",";
    j += "\"rot\":" + String(robot.toolRotateDeg, 1) + ",";
    j += "\"grip\":" + String(robot.gripperDeg, 1) + ",";
    j += "\"act_a1\":" + String(robot.actualJoints.arm1Deg, 2) + ",";
    j += "\"act_a2\":" + String(robot.actualJoints.arm2Deg, 2) + ",";
    j += "\"act_z\":" + String(robot.actualJoints.zMm, 2) + ",";
    j += "\"act_x\":" + String(robot.actualTcp.xMm, 1) + ",";
    j += "\"act_y\":" + String(robot.actualTcp.yMm, 1) + ",";
    j += "\"mode\":\"" + String(robot.operatingMode == OperatingMode::AUTO ? "AUTO" : "MANUAL") + "\",";
    j += "\"auto\":" + String(static_cast<uint8_t>(robot.autoState)) + ",";
    j += "\"estop\":" + String(robot.estopLatched ? "true" : "false") + ",";
    j += "\"running\":" + String(axesRunning() ? "true" : "false") + ",";
    j += "\"gpad\":" + String(robot.controllerConnected ? "true" : "false") + ",";
    j += "\"col_act\":" + String(columnCycleActive ? "true" : "false") + ",";
    j += "\"col_id\":" + String(currentColumnId) + ",";
    j += "\"points\":" + String(programPointCount);
    j += "}";
    sendJson(200, j);
}

void handleSet()
{
    if (!webGuard()) return;

    jogContinuousActive = false;
    padHoming = false;
    stopColumnCycle();

    JointPosition t = robot.joints;
    if (server.hasArg("l1")) t.arm1Deg = server.arg("l1").toFloat();
    if (server.hasArg("l2")) t.arm2Deg = server.arg("l2").toFloat();
    if (server.hasArg("z")) t.zMm = server.arg("z").toFloat();

    if (server.hasArg("rot"))
        robot.toolRotateDeg = clampFloat(server.arg("rot").toFloat(), TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
    if (server.hasArg("g"))
        robot.gripperDeg = clampFloat(server.arg("g").toFloat(), GRIP_MIN_DEG, GRIP_MAX_DEG);

    if (server.hasArg("l1") || server.hasArg("l2") || server.hasArg("z"))
        commandWebJointPosition(t);

    sendJson(200, "{\"status\":\"ok\"}");
}

void handleJog()
{
    if (!webGuard()) return;
    if (!server.hasArg("axis"))
    {
        sendJson(400, "{\"error\":\"немає axis\"}");
        return;
    }

    padHoming = false;
    stopColumnCycle();

    const String axis = server.arg("axis");
    const float dir = (server.hasArg("dir") && server.arg("dir").toFloat() < 0.0f) ? -1.0f : 1.0f;
    const float step = server.hasArg("step") ? server.arg("step").toFloat() : 0.0f;

    JointPosition t = robot.joints; // база — команда, тож швидкі кліки складаються

    if (step != 0.0f)
    {
        jogContinuousActive = false;
        if (axis == "z") t.zMm += step;
        else if (axis == "l1") t.arm1Deg += step;
        else if (axis == "l2") t.arm2Deg += step;
        else if (axis == "rot")
        {
            robot.toolRotateDeg = clampFloat(robot.toolRotateDeg + step, TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
            sendJson(200, "{\"status\":\"ok\"}");
            return;
        }
        commandWebJointPosition(t);
        sendJson(200, "{\"status\":\"ok\"}");
        return;
    }

    // Неперервний джог: їдемо до ліміту, поки веб повторює команду
    if (axis == "rot")
    {
        robot.toolRotateDeg = clampFloat(robot.toolRotateDeg + dir * 2.0f, TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
        sendJson(200, "{\"status\":\"ok\"}");
        return;
    }

    updateActualRobotPosition();
    bool atLimit = false;
    if (axis == "z")
    {
        atLimit = (dir > 0 && robot.actualJoints.zMm >= Z_MAX_MM) || (dir < 0 && robot.actualJoints.zMm <= Z_MIN_MM);
        t = robot.actualJoints;
        t.zMm = (dir > 0) ? Z_MAX_MM : Z_MIN_MM;
    }
    else if (axis == "l1")
    {
        atLimit = (dir > 0 && robot.actualJoints.arm1Deg >= ARM1_MAX_DEG) || (dir < 0 && robot.actualJoints.arm1Deg <= ARM1_MIN_DEG);
        t = robot.actualJoints;
        t.arm1Deg = (dir > 0) ? ARM1_MAX_DEG : ARM1_MIN_DEG;
    }
    else if (axis == "l2")
    {
        atLimit = (dir > 0 && robot.actualJoints.arm2Deg >= ARM2_MAX_DEG) || (dir < 0 && robot.actualJoints.arm2Deg <= ARM2_MIN_DEG);
        t = robot.actualJoints;
        t.arm2Deg = (dir > 0) ? ARM2_MAX_DEG : ARM2_MIN_DEG;
    }
    else
    {
        sendJson(400, "{\"error\":\"невідома вісь\"}");
        return;
    }

    if (atLimit)
    {
        jogContinuousActive = false;
        sendJson(200, "{\"status\":\"limit\"}");
        return;
    }

    jogContinuousActive = true;
    jogWatchdogMs = millis() + JOG_WATCHDOG_MS;
    commandWebJointPosition(t);
    sendJson(200, "{\"status\":\"ok\"}");
}

void handleJogStop()
{
    jogContinuousActive = false;
    emergencyStopMotion();
    sendJson(200, "{\"status\":\"stopped\"}");
}

// Стоп: зупиняє все. В AUTO — ставить на паузу (можна продовжити), в MANUAL — просто стоп.
void handleStop()
{
    jogContinuousActive = false;
    if (robot.operatingMode == OperatingMode::AUTO)
        pauseAuto("WEB СТОП — AUTO ПАУЗА");
    emergencyStopMotion();
    sendJson(200, "{\"status\":\"stopped\"}");
}

void handleHome()
{
    if (!webGuard()) return;
    jogContinuousActive = false;
    padHoming = false;
    stopColumnCycle();
    const JointPosition home = {HOME_ARM1_DEG, HOME_ARM2_DEG, HOME_Z_MM};
    commandWebJointPosition(home);
    robot.toolRotateDeg = HOME_TOOL_ROTATE_DEG;
    robot.gripperDeg = HOME_GRIP_DEG;
    sendJson(200, "{\"status\":\"homing\"}");
}

void handleColumn()
{
    if (!webGuard()) return;

    uint8_t mode = 0;
    if (server.hasArg("id"))
    {
        const int v = server.arg("id").toInt(); // "all" -> 0
        if (v >= 1 && v <= 3) mode = static_cast<uint8_t>(v);
    }
    jogContinuousActive = false;
    padHoming = false;
    startColumnCycle(mode);
    sendJson(200, "{\"status\":\"started\"}");
}

void handleColumnStop()
{
    stopColumnCycle();
    sendJson(200, "{\"status\":\"stopped\"}");
}

void handleTeach()
{
    if (!webGuard()) return;
    teachCurrentPoint();
    sendJson(200, "{\"status\":\"ok\",\"points\":" + String(programPointCount) + "}");
}

void handleClear()
{
    if (!webGuard()) return;
    clearProgram();
    sendJson(200, "{\"status\":\"ok\",\"points\":" + String(programPointCount) + "}");
}

void handleAuto()
{
    if (robot.estopLatched)
    {
        sendJson(403, "{\"error\":\"E-STOP активний\"}");
        return;
    }
    if (robot.operatingMode != OperatingMode::AUTO)
    {
        sendJson(409, "{\"error\":\"Спочатку перейдіть в режим AUTO\"}");
        return;
    }
    toggleAutoRunPause();
    sendJson(200, "{\"status\":\"ok\",\"auto_state\":" + String(static_cast<uint8_t>(robot.autoState)) + "}");
}

void handleMode()
{
    toggleOperatingMode();
    sendJson(200, "{\"status\":\"ok\",\"mode\":\"" +
                      String(robot.operatingMode == OperatingMode::AUTO ? "AUTO" : "MANUAL") + "\"}");
}

// ============================================================
// Wi-Fi
// ============================================================
void onWifiEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.printf("[WiFi] клієнт підключився (усього %d)\n", WiFi.softAPgetStationNum());
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        Serial.printf("[WiFi] клієнт відключився (усього %d)\n", WiFi.softAPgetStationNum());
        break;
    default:
        break;
    }
}

void startWifiAp()
{
    // ВАЖЛИВО: Wi-Fi піднімаємо ДО Bluetooth, без setSleep(false) і без max TX power.
    WiFi.persistent(false);
    WiFi.onEvent(onWifiEvent);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    const bool ok = WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, 0, AP_MAX_CLIENTS);
    Serial.printf("Wi-Fi AP '%s' пароль '%s' канал %u: %s  IP %s\n",
                  AP_SSID, AP_PASS, AP_CHANNEL, ok ? "OK" : "ПОМИЛКА",
                  WiFi.softAPIP().toString().c_str());
}

const char *resetReasonText(esp_reset_reason_t r)
{
    switch (r)
    {
    case ESP_RST_POWERON: return "живлення увімкнено";
    case ESP_RST_SW: return "програмний ресет";
    case ESP_RST_PANIC: return "PANIC (збій програми)";
    case ESP_RST_INT_WDT: return "watchdog (interrupt)";
    case ESP_RST_TASK_WDT: return "watchdog (task)";
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_BROWNOUT: return "BROWNOUT (просідає живлення!)";
    default: return "інше";
    }
}

// ============================================================
// SETUP
// ============================================================
void setup()
{
    Serial.begin(115200);
    delay(400);
    Serial.println();
    Serial.printf("Причина старту: %s\n", resetReasonText(esp_reset_reason()));

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

    robot.controlMode = ControlMode::JOINT;
    robot.operatingMode = OperatingMode::MANUAL;
    robot.autoState = AutoState::IDLE;
    robot.estopLatched = false;
    robot.controllerConnected = false;
    robot.gamepadArmed = false;

    robot.joints = {HOME_ARM1_DEG, HOME_ARM2_DEG, HOME_Z_MM};
    robot.actualJoints = robot.joints;
    robot.tcp = forwardKinematics(robot.joints);
    robot.actualTcp = robot.tcp;
    robot.toolRotateDeg = HOME_TOOL_ROTATE_DEG;
    robot.gripperDeg = HOME_GRIP_DEG;

    servoRotate.setPeriodHertz(50);
    servoGrip.setPeriodHertz(50);
    servoRotate.attach(PIN_SERVO_ROTATE, SERVO_ROTATE_MIN_US, SERVO_ROTATE_MAX_US);
    servoGrip.attach(PIN_SERVO_GRIP, SERVO_GRIP_MIN_US, SERVO_GRIP_MAX_US);

    servoRotateFilteredDeg = robot.toolRotateDeg;
    servoGripFilteredDeg = robot.gripperDeg;
    servoRotate.writeMicroseconds(angleToPulseUs(servoRotateFilteredDeg, SERVO_ROTATE_MIN_US, SERVO_ROTATE_MAX_US));
    servoGrip.writeMicroseconds(angleToPulseUs(servoGripFilteredDeg, SERVO_GRIP_MIN_US, SERVO_GRIP_MAX_US));

    if (!initializeSteppers())
    {
        digitalWrite(PIN_LED_ERROR, HIGH);
        Serial.println("Помилка ініціалізації FastAccelStepper");
        while (true) delay(1000);
    }
    updateActualRobotPosition();

    preferences.begin("scara-auto", false);
    loadProgram();

    startWifiAp(); // ДО Bluepad32

#if ENABLE_GAMEPAD
    BP32.setup(&onConnectedController, &onDisconnectedController);
    Serial.println("Bluetooth-геймпад: увімкнено");
#else
    Serial.println("Bluetooth-геймпад: ВИМКНЕНО (ENABLE_GAMEPAD=0)");
#endif

    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/state", HTTP_GET, handleState);
    server.on("/set", HTTP_GET, handleSet);
    server.on("/jog", HTTP_GET, handleJog);
    server.on("/jog_stop", HTTP_GET, handleJogStop);
    server.on("/stop", HTTP_GET, handleStop);
    server.on("/home", HTTP_GET, handleHome);
    server.on("/teach", HTTP_GET, handleTeach);
    server.on("/clear", HTTP_GET, handleClear);
    server.on("/auto", HTTP_GET, handleAuto);
    server.on("/mode", HTTP_GET, handleMode);
    server.on("/column", HTTP_GET, handleColumn);
    server.on("/column_stop", HTTP_GET, handleColumnStop);
    server.begin();
    Serial.println("Веб-панель: http://192.168.4.1");

    Serial.printf("Завантажено точок: %u\n", programPointCount);
    printCurrentPosition();
    lastControlMs = millis();
}

// ============================================================
// LOOP
// ============================================================
void loop()
{
#if ENABLE_GAMEPAD
    BP32.update();
#endif
    server.handleClient();

    updateEmergencyStop();
    updatePanelButtons();
    updateActualRobotPosition();
    refreshServos();

    // Сторожовий таймер неперервного джогу: веб перестав слати — стоп
    if (jogContinuousActive && millis() > jogWatchdogMs)
    {
        jogContinuousActive = false;
        emergencyStopMotion();
        Serial.println("JOG: watchdog — автозупинка");
    }

    // Синхронізація команди з фактом після зупинки осей
    static bool wasRunning = false;
    const bool isRunning = axesRunning();
    if (wasRunning && !isRunning) syncCommandToActual();
    wasRunning = isRunning;

    const uint32_t now = millis();
    if (now - lastControlMs >= CONTROL_PERIOD_MS)
    {
        float dt = static_cast<float>(now - lastControlMs) / 1000.0f;
        lastControlMs = now;
        dt = clampFloat(dt, 0.0f, MAX_CONTROL_DT_S);

        if (!robot.estopLatched)
        {
            processGamepad(dt);
            processAuto();
            processColumnCycle();
        }
    }

    // Діагностика Wi-Fi/пам'яті раз на 5 с
    static uint32_t lastDiag = 0;
    if (now - lastDiag >= DIAG_PERIOD_MS)
    {
        lastDiag = now;
        Serial.printf("[diag] клієнтів Wi-Fi: %d  heap: %u  геймпад: %s  режим: %s\n",
                      WiFi.softAPgetStationNum(), ESP.getFreeHeap(),
                      robot.controllerConnected ? "так" : "ні",
                      robot.operatingMode == OperatingMode::AUTO ? "AUTO" : "MANUAL");
    }

    delay(1);
}