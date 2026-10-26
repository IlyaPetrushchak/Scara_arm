#pragma once
/* kinematics.h — пряма та обернена кінематика SCARA (2 плеча в площині XY + Z). */
#include "types.h"

CartesianPosition forwardKinematics(const JointPosition &j);
// elbowUp=true -> A2 >= 0; false -> A2 < 0. out.zMm береться з поточної команди.
bool inverseKinematics(float xMm, float yMm, bool elbowUp, JointPosition &out);
