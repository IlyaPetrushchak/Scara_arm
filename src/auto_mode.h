#pragma once
/* auto_mode.h — автоматичне відтворення збереженої програми. */

extern bool autoMotionStarted;

void setAutoFault(const char *reason);
void pauseAuto(const char *reason);
void startAuto();
void toggleAutoRunPause();
void processAuto(); // щоцикл
