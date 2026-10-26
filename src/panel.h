#pragma once
/* panel.h — фізична панель: E-STOP, кнопки MODE/TEACH, світлодіоди, перемикання режимів. */

void initPanel(); // піни, світлодіоди, початковий стан кнопок
void updateEmergencyStop();
void updatePanelButtons();
void toggleControlMode();   // JOINT <-> TOOL (геймпад)
void toggleOperatingMode(); // MANUAL <-> AUTO
void printCurrentPosition();
