#include "state.h"
#include "config.h"
#include "kinematics.h"

RobotState robot;

void initRobotState()
{
    robot.controlMode = ControlMode::JOINT;
    robot.operatingMode = OperatingMode::MANUAL;
    robot.autoState = AutoState::IDLE;
    robot.estopLatched = false;
    robot.controllerConnected = false;
    robot.gamepadArmed = false;

    robot.joints = {HOME_ARM1_DEG, HOME_ARM2_DEG, HOME_Z_MM};
    robot.actualJoints = robot.joints;
    robot.tcp = forwardKinematics(robot.joints);
    robot.actualTcp = robot.tcp;
    robot.toolRotateDeg = HOME_TOOL_ROTATE_DEG;
    robot.gripperDeg = HOME_GRIP_DEG;
}
