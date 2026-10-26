#pragma once
/* steppers.h — драйвери крокових двигунів (FastAccelStepper): осі A1, A2, Z. */
#include <FastAccelStepper.h>

extern FastAccelStepper *stepperArm1;
extern FastAccelStepper *stepperArm2;
extern FastAccelStepper *stepperZ;

// false -> не вдалося підключити пін(и). Ручний HOME: поточне положення = HOME.
bool initializeSteppers();
