#include "column_cycle.h"
#include "config.h"
#include "state.h"
#include "kinematics.h"
#include "motion.h"

enum class ColumnStep : uint8_t { IDLE, GOTO_TOP, DESCEND, DWELL, ASCEND, NEXT_COL };

bool columnCycleActive = false;
ColumnStep columnStep = ColumnStep::IDLE;
uint8_t currentColumnId = 0;  // 1..3
uint8_t targetColumnMode = 0; // 0 = усі, 1..3 = одна
uint32_t columnDwellTimer = 0;

void startColumnCycle(uint8_t mode)
{
    if (robot.estopLatched) return;
    targetColumnMode = mode;
    currentColumnId = (mode >= 1 && mode <= 3) ? mode : 1;
    columnStep = ColumnStep::GOTO_TOP;
    columnCycleActive = true;
    columnDwellTimer = 0;
    Serial.printf("КОЛОНКИ: старт, режим %u\n", targetColumnMode);
}

void stopColumnCycle()
{
    if (!columnCycleActive) return;
    columnCycleActive = false;
    columnStep = ColumnStep::IDLE;
    columnDwellTimer = 0;
    Serial.println("КОЛОНКИ: зупинено");
}

void processColumnCycle()
{
    if (!columnCycleActive || robot.estopLatched) return;
    if (!motionFinished()) return;

    const uint8_t idx = currentColumnId - 1;
    JointPosition col;
    if (idx > 2 || !inverseKinematics(COLUMN_X_MM[idx], COLUMN_Y_MM[idx], true, col))
    {
        Serial.printf("КОЛОНКИ: IK недосяжна, колонка %u\n", currentColumnId);
        stopColumnCycle();
        return;
    }

    const uint32_t now = millis();

    switch (columnStep)
    {
    case ColumnStep::GOTO_TOP:
        col.zMm = COLUMN_Z_TOP_MM;
        commandCoordinatedJointPosition(col, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
        columnStep = ColumnStep::DESCEND;
        break;

    case ColumnStep::DESCEND:
        col.zMm = COLUMN_Z_BOTTOM_MM;
        commandCoordinatedJointPosition(col, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
        columnStep = ColumnStep::DWELL;
        columnDwellTimer = 0;
        break;

    case ColumnStep::DWELL:
        if (columnDwellTimer == 0)
            columnDwellTimer = now;
        else if (now - columnDwellTimer >= COLUMN_DWELL_MS)
        {
            columnDwellTimer = 0;
            columnStep = ColumnStep::ASCEND;
        }
        break;

    case ColumnStep::ASCEND:
        col.zMm = COLUMN_Z_TOP_MM;
        commandCoordinatedJointPosition(col, AUTO_ARM1_SPEED_DEG_S, AUTO_ARM2_SPEED_DEG_S, AUTO_Z_SPEED_MM_S);
        columnStep = ColumnStep::NEXT_COL;
        break;

    case ColumnStep::NEXT_COL:
        if (targetColumnMode >= 1 && targetColumnMode <= 3)
            stopColumnCycle();
        else if (currentColumnId < 3)
        {
            currentColumnId++;
            columnStep = ColumnStep::GOTO_TOP;
        }
        else
        {
            Serial.println("КОЛОНКИ: усі 3 пройдено");
            stopColumnCycle();
        }
        break;

    default:
        stopColumnCycle();
        break;
    }
}
