#include "gamepad.h"
#include "config.h"
#include "state.h"
#include "utils.h"
#include "kinematics.h"
#include "motion.h"
#include "column_cycle.h"
#include "auto_mode.h"
#include "panel.h"
#include "servos.h"
#include "steppers.h"
#if ENABLE_GAMEPAD
#include <Bluepad32.h>
#endif

uint32_t lastGamepadPacketMs = 0;
uint32_t gamepadNeutralSinceMs = 0;
bool previousPadStart = false;
bool previousPadY = false; // Y на геймпаді (повернення в HOME)

#if ENABLE_GAMEPAD

ControllerPtr controllers[BP32_MAX_GAMEPADS];

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
           !c->a() && !c->b() && !c->y() && !c->miscStart() &&
           c->throttle() < 100 && c->brake() < 100;
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

    // Схват: R2 = закрити, L2 = відкрити (плавно, пропорційно натиску);
    // кнопки A (закрити) / B (відкрити) працюють як і раніше.
    float grip = (static_cast<float>(c->throttle()) - static_cast<float>(c->brake())) / 1023.0f;
    if (fabsf(grip) < 0.08f) grip = 0.0f;
    if (c->a()) grip = 1.0f;
    if (c->b()) grip = -1.0f;
    grip = clampFloat(grip, -1.0f, 1.0f);

    float rotCmd = rx;
    const float zCmd = ry;
    if (fabsf(ry) > 0.15f && fabsf(rx) < 0.30f) rotCmd = 0.0f; // Z без випадкового повороту

    // Під час повернення в HOME ручне керування серв не втручається в ціль
    // Викликаємо щоцикл, щоб команда схвата плавно гальмувала після відпускання
    if (!padHoming) updateServoTargets(rotCmd, grip, dt);

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

void initGamepad()
{
    BP32.setup(&onConnectedController, &onDisconnectedController);
    Serial.println("Bluetooth-геймпад: увімкнено");
}

void updateGamepadBluetooth() { BP32.update(); }

#else // ENABLE_GAMEPAD == 0

void updateControllerConnectedFlag() { robot.controllerConnected = false; }
void processGamepad(float) {}
void initGamepad() { Serial.println("Bluetooth-геймпад: ВИМКНЕНО (ENABLE_GAMEPAD=0)"); }
void updateGamepadBluetooth() {}

#endif
