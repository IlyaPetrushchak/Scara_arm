#pragma once
/* state.h — єдиний глобальний стан робота (позиції, режими, прапорці). */
#include "types.h"

extern RobotState robot;

// Початкові значення (HOME, режими, прапорці). Викликається один раз у setup().
void initRobotState();
