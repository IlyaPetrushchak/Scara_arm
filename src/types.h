#pragma once
/* types.h — спільні типи даних робота. */
#include <Arduino.h>

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
