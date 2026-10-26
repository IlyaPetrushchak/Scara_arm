#pragma once
/* utils.h — прості математичні помічники та перерахунок градуси/мм <-> кроки. */
#include <Arduino.h>

float clampFloat(float v, float lo, float hi);
uint32_t clampStepHz(float hz, uint32_t maxHz);
float degToRad(float d);
float radToDeg(float r);
float normalizeJoystick(int32_t value); // мертва зона + крива x^1.5

int32_t arm1DegreesToSteps(float a);
int32_t arm2DegreesToSteps(float a);
int32_t zMillimetersToSteps(float z);
float arm1StepsToDegrees(int32_t s);
float arm2StepsToDegrees(int32_t s);
float zStepsToMillimeters(int32_t s);
