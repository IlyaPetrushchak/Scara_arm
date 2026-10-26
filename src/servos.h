#pragma once
/* servos.h — серви кисті (поворот) і схвата з фільтрацією/обмеженням швидкості. */

void initServos();
int angleToPulseUs(float angleDeg, int minUs, int maxUs);
void updateServoTargets(float rotateCmd, float gripCmd, float dt); // геймпад
void refreshServos();                                               // щоцикл loop
