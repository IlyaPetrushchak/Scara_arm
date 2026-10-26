#pragma once
/* column_cycle.h — цикл «над колонкою, вниз, пауза, вгору» для 3 колонок. */
#include <Arduino.h>

extern bool columnCycleActive;
extern uint8_t currentColumnId; // 1..3

void startColumnCycle(uint8_t mode); // 0 = усі, 1..3 = одна
void stopColumnCycle();
void processColumnCycle();           // щоцикл
