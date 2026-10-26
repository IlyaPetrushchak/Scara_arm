#pragma once
/* gamepad.h — Bluetooth-геймпад (Bluepad32). З -DENABLE_GAMEPAD=0 усе стає заглушками. */
#include <Arduino.h>
#include "config.h"

extern uint32_t lastGamepadPacketMs;
extern uint32_t gamepadNeutralSinceMs;
extern bool previousPadStart;
extern bool previousPadY;

void initGamepad(); // реєструє Bluepad32 (викликати ПІСЛЯ підняття Wi-Fi)
void updateControllerConnectedFlag();
void processGamepad(float dt); // щоцикл
void updateGamepadBluetooth(); // BP32.update() — у loop
