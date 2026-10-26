#pragma once
/* motion.h — позиція осей і команди руху: PTP, ручний (геймпад), плавний веб-рух, зупинки. */
#include "types.h"

// Прапорці веб-руху / джогу (використовують веб, геймпад, панель)
extern bool padHoming;               // іде повернення в HOME з геймпада
extern uint32_t jogWatchdogMs;
extern bool jogContinuousActive;
extern bool webFollow;               // веб веде осі до webTarget
extern JointPosition webTarget;
extern float webRotDir;              // утримуваний поворот кисті: -1 / 0 / +1
extern uint32_t webRotWatchdogMs;

void updateActualRobotPosition();
bool axesRunning();
bool motionFinished();
void syncCommandToActual();
JointPosition clampJointPosition(const JointPosition &t);

void emergencyStopMotion(); // миттєва зупинка
void softStopMotion();      // зупинка з гальмуванням по рампі

void commandManualJointPosition(const JointPosition &target, float dt); // геймпад
void commandCoordinatedJointPosition(const JointPosition &target, float maxSpeed1, float maxSpeed2, float maxSpeedZ);
void commandAutoJointPosition(const JointPosition &t);
void commandWebJointPosition(const JointPosition &t);

void startWebFollow(const JointPosition &t); // веб ставить ціль
void processWebMotion(float dt);             // щоцикл (10 мс)
