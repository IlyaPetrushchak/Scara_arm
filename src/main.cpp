/*
 * SCARA Robot Controller
 * ------------------------------------------------------------
 * 3-осьовий SCARA: геймпад (Bluepad32) + веб-панель по Wi-Fi SoftAP,
 * TEACH-точки в NVS, автоматичне відтворення, 3 колонки.
 *
 * Модулі (src/):
 *   config.h        — усі налаштування (піни, механіка, ліміти, швидкості, Wi-Fi)
 *   types.h         — спільні типи          state.*       — глобальний стан робота
 *   utils.*         — математика/кроки      kinematics.*  — кінематика
 *   steppers.*      — крокові двигуни       servos.*      — серви кисті/схвата
 *   motion.*        — команди руху, плавний веб-рух, зупинки
 *   column_cycle.*  — цикл 3 колонок        program.*     — TEACH-точки в NVS
 *   auto_mode.*     — автоматичний режим    panel.*       — E-STOP, кнопки, режими
 *   gamepad.*       — Bluetooth-геймпад     wifi_ap.*     — Wi-Fi AP + DNS
 *   web_server.*    — HTTP + WebSocket      web_ui.h      — HTML веб-панелі
 *
 * HOME ручний: перед подачею живлення / ресетом постав руку в HOME (config.h).
 * Якщо Wi-Fi нестабільний — збери з -DENABLE_GAMEPAD=0 (build_flags).
 */
#include <Arduino.h>
#include <WiFi.h>
#include <esp_system.h>

#include "config.h"
#include "state.h"
#include "utils.h"
#include "motion.h"
#include "steppers.h"
#include "servos.h"
#include "program.h"
#include "auto_mode.h"
#include "column_cycle.h"
#include "panel.h"
#include "gamepad.h"
#include "wifi_ap.h"
#include "web_server.h"

static uint32_t lastControlMs = 0;

const char *resetReasonText(esp_reset_reason_t r)
{
    switch (r)
    {
    case ESP_RST_POWERON: return "живлення увімкнено";
    case ESP_RST_SW: return "програмний ресет";
    case ESP_RST_PANIC: return "PANIC (збій програми)";
    case ESP_RST_INT_WDT: return "watchdog (interrupt)";
    case ESP_RST_TASK_WDT: return "watchdog (task)";
    case ESP_RST_WDT: return "watchdog";
    case ESP_RST_BROWNOUT: return "BROWNOUT (просідає живлення!)";
    default: return "інше";
    }
}

void setup()
{
    Serial.begin(115200);
    delay(400);
    Serial.println();
    Serial.printf("Причина старту: %s\n", resetReasonText(esp_reset_reason()));

    initPanel();
    initRobotState();
    initServos();

    if (!initializeSteppers())
    {
        digitalWrite(PIN_LED_ERROR, HIGH);
        Serial.println("Помилка ініціалізації FastAccelStepper");
        while (true) delay(1000);
    }
    updateActualRobotPosition();

    initProgramStorage();

    startWifiAp(); // Wi-Fi піднімаємо ДО Bluetooth
    initGamepad();
    setupWebServer();

    Serial.printf("Завантажено точок: %u\n", programPointCount);
    printCurrentPosition();
    lastControlMs = millis();
}

void loop()
{
    processWifiDns();
    updateGamepadBluetooth();
    processWebServer();

    updateEmergencyStop();
    updatePanelButtons();
    updateActualRobotPosition();
    refreshServos();

    // Сторожовий таймер неперервного джогу: веб перестав слати — стоп
    if (jogContinuousActive && millis() > jogWatchdogMs)
    {
        softStopMotion();
        Serial.println("JOG: watchdog — автозупинка");
    }

    // Синхронізація команди з фактом після зупинки осей
    static bool wasRunning = false;
    const bool isRunning = axesRunning();
    if (wasRunning && !isRunning && !webFollow) syncCommandToActual();
    wasRunning = isRunning;

    const uint32_t now = millis();
    if (now - lastControlMs >= CONTROL_PERIOD_MS)
    {
        float dt = static_cast<float>(now - lastControlMs) / 1000.0f;
        lastControlMs = now;
        dt = clampFloat(dt, 0.0f, MAX_CONTROL_DT_S);

        if (!robot.estopLatched)
        {
            processWebMotion(dt);
            processGamepad(dt);
            processAuto();
            processColumnCycle();
        }
    }

    pushWebState(now);

    // Діагностика Wi-Fi/пам'яті раз на 5 с
    static uint32_t lastDiag = 0;
    if (now - lastDiag >= DIAG_PERIOD_MS)
    {
        lastDiag = now;
        Serial.printf("[diag] клієнтів Wi-Fi: %d  heap: %u  геймпад: %s  режим: %s\n",
                      WiFi.softAPgetStationNum(), ESP.getFreeHeap(),
                      robot.controllerConnected ? "так" : "ні",
                      robot.operatingMode == OperatingMode::AUTO ? "AUTO" : "MANUAL");
    }

    delay(1);
}