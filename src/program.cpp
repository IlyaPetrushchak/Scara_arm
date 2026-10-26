#include "program.h"
#include <Preferences.h>
#include "state.h"
#include "motion.h"

Preferences preferences;
ProgramPoint programPoints[MAX_PROGRAM_POINTS];
uint8_t programPointCount = 0;
uint8_t currentProgramPoint = 0;

void saveProgram()
{
    preferences.putUChar("count", programPointCount);
    if (programPointCount > 0)
        preferences.putBytes("points", programPoints, sizeof(ProgramPoint) * programPointCount);
    else if (preferences.isKey("points"))
        preferences.remove("points");
}

void loadProgram()
{
    programPointCount = preferences.getUChar("count", 0);
    if (programPointCount > MAX_PROGRAM_POINTS)
    {
        programPointCount = 0;
        saveProgram();
        return;
    }
    if (programPointCount > 0)
    {
        const size_t expected = sizeof(ProgramPoint) * programPointCount;
        if (preferences.getBytesLength("points") != expected ||
            preferences.getBytes("points", programPoints, expected) != expected)
        {
            programPointCount = 0;
            saveProgram();
        }
    }
}

void clearProgram()
{
    if (robot.operatingMode != OperatingMode::MANUAL || axesRunning()) return;
    programPointCount = 0;
    currentProgramPoint = 0;
    saveProgram();
    Serial.println("TEACH: програму очищено");
}

void teachCurrentPoint()
{
    if (robot.estopLatched || robot.operatingMode != OperatingMode::MANUAL) return;
    if (axesRunning())
    {
        Serial.println("TEACH: спочатку зупини осі");
        return;
    }
    if (programPointCount >= MAX_PROGRAM_POINTS)
    {
        Serial.println("TEACH: програма заповнена");
        return;
    }

    updateActualRobotPosition();
    ProgramPoint &p = programPoints[programPointCount];
    p.joints = robot.actualJoints;
    p.toolRotateDeg = robot.toolRotateDeg;
    p.gripperDeg = robot.gripperDeg;
    p.dwellMs = DEFAULT_POINT_DWELL_MS;

    programPointCount++;
    saveProgram();

    Serial.printf("TEACH: P%02u  A1=%.2f A2=%.2f Z=%.2f ROT=%.1f GRIP=%.1f\n",
                  programPointCount, p.joints.arm1Deg, p.joints.arm2Deg, p.joints.zMm,
                  p.toolRotateDeg, p.gripperDeg);
}

void initProgramStorage()
{
    preferences.begin("scara-auto", false);
    loadProgram();
}
