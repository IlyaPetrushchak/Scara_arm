#pragma once
/* program.h — TEACH-програма (точки) і її збереження в NVS. */
#include "types.h"
#include "config.h"

extern ProgramPoint programPoints[MAX_PROGRAM_POINTS];
extern uint8_t programPointCount;
extern uint8_t currentProgramPoint;

void initProgramStorage(); // відкрити NVS і завантажити збережені точки
void saveProgram();
void loadProgram();
void clearProgram();
void teachCurrentPoint();
