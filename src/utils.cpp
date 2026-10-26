#include "utils.h"
#include "config.h"

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
