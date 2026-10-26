#include "motion.h"
#include "config.h"
#include "state.h"
#include "utils.h"
#include "kinematics.h"
#include "steppers.h"
#include "column_cycle.h"

// ---- Прапорці руху ----
bool padHoming = false;
uint32_t jogWatchdogMs = 0;
bool jogContinuousActive = false;

// Плавний веб-рух: веб ставить ЦІЛЬ, а прошивка щоцикл (10 мс) веде осі до неї
// з постійною швидкістю і м'яким підходом (як геймпад) -> без ривків між командами.
bool webFollow = false;
JointPosition webTarget = {0.0f, 0.0f, 0.0f};
float webRotDir = 0.0f;
uint32_t webRotWatchdogMs = 0;

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
    webFollow = false;
    webRotDir = 0.0f;
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

    webFollow = false; // геймпад важливіший за веб
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

    webFollow = false;
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

// М'яка зупинка (з гальмуванням по рампі драйвера)
void softStopMotion()
{
    webFollow = false;
    jogContinuousActive = false;
    if (stepperArm1) stepperArm1->stopMove();
    if (stepperArm2) stepperArm2->stopMove();
    if (stepperZ) stepperZ->stopMove();
}

void startWebFollow(const JointPosition &t)
{
    if (robot.estopLatched) return;
    webTarget = clampJointPosition(t);
    webFollow = true;
}

// Викликається щоцикл (10 мс). Уставка robot.joints їде до webTarget плавно.
void processWebMotion(float dt)
{
    if (robot.operatingMode != OperatingMode::MANUAL)
    {
        webFollow = false;
        webRotDir = 0.0f;
        return;
    }
    if (dt <= 0.0f) return;

    if (webRotDir != 0.0f)
    {
        if (millis() > webRotWatchdogMs) webRotDir = 0.0f;
        else
            robot.toolRotateDeg = clampFloat(robot.toolRotateDeg + webRotDir * WEB_ROT_SPEED_DEG_S * dt,
                                             TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
    }

    if (!webFollow) return;

    const JointPosition cur = robot.joints;
    const float e1 = webTarget.arm1Deg - cur.arm1Deg;
    const float e2 = webTarget.arm2Deg - cur.arm2Deg;
    const float ez = webTarget.zMm - cur.zMm;

    const float maxS1 = ARM1_JOG_SPEED_DEG_S;
    const float maxS2 = ARM2_JOG_SPEED_DEG_S;
    const float maxSz = Z_JOG_SPEED_MM_S;

    // час, за який найповільніша вісь дійшла б до цілі на максимальній швидкості
    const float dist = fmax(fabsf(e1), fmaxf(fabsf(e2), fabsf(ez))); 

    if (dist < 0.001f)
    {
        // дійшли: ставимо точну ціль і лишаємо драйверу довести її до кінця
        webFollow = false;
        const int32_t s1 = arm1DegreesToSteps(webTarget.arm1Deg);
        const int32_t s2 = arm2DegreesToSteps(webTarget.arm2Deg);
        const int32_t sz = zMillimetersToSteps(webTarget.zMm);
        if (stepperArm1->targetPos() != s1) stepperArm1->moveTo(s1);
        if (stepperArm2->targetPos() != s2) stepperArm2->moveTo(s2);
        if (stepperZ->targetPos() != sz) stepperZ->moveTo(sz);
        robot.joints = webTarget;
        robot.tcp = forwardKinematics(robot.joints);
        return;
    }

    // далеко від цілі — максимальна швидкість, біля цілі — експоненційне гальмування
    const float StepMax = maxS1 * dt;
    const float scale = (dist > StepMax) ? (StepMax / dist) : 1.0f;

    const JointPosition next = {
        cur.arm1Deg + e1 * scale,
        cur.arm2Deg + e2 * scale,
        cur.zMm + ez * scale};

    const uint32_t m1 = clampStepHz(ARM1_JOG_SPEED_DEG_S * STEPS_PER_DEG_ARM1, ARM1_MAX_STEP_HZ);
    const uint32_t m2 = clampStepHz(ARM2_JOG_SPEED_DEG_S * STEPS_PER_DEG_ARM2, ARM2_MAX_STEP_HZ);
    const uint32_t mz = clampStepHz(Z_JOG_SPEED_MM_S * STEPS_PER_MM_Z, Z_MAX_STEP_HZ);

    const float d1 = fabsf(e1 * scale), d2 = fabsf(e2 * scale), dz = fabsf(ez * scale);

    if (d1 > 0.0001f) stepperArm1->setSpeedInHz(clampStepHz(d1 / dt * STEPS_PER_DEG_ARM1, m1));
    if (d2 > 0.0001f) stepperArm2->setSpeedInHz(clampStepHz(d2 / dt * STEPS_PER_DEG_ARM2, m2));
    if (dz > 0.0001f) stepperZ->setSpeedInHz(clampStepHz(dz / dt * STEPS_PER_MM_Z, mz));

    const int32_t s1 = arm1DegreesToSteps(next.arm1Deg);
    const int32_t s2 = arm2DegreesToSteps(next.arm2Deg);
    const int32_t sz = zMillimetersToSteps(next.zMm);
    
    if (stepperArm1->targetPos() != s1) stepperArm1->moveTo(s1);
    if (stepperArm2->targetPos() != s2) stepperArm2->moveTo(s2);
    if (stepperZ->targetPos() != sz) stepperZ->moveTo(sz);

    robot.joints = next;
    robot.tcp = forwardKinematics(robot.joints);
}
