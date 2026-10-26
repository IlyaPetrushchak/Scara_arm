#include "kinematics.h"
#include "config.h"
#include "state.h"
#include "utils.h"

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
