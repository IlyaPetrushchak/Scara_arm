#include "steppers.h"
#include "config.h"
#include "utils.h"

FastAccelStepperEngine stepperEngine;
FastAccelStepper *stepperArm1 = nullptr;
FastAccelStepper *stepperArm2 = nullptr;
FastAccelStepper *stepperZ = nullptr;

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
