#pragma once
/*
 * config.h — ВСІ налаштування в одному місці: піни, механіка, ліміти, швидкості, таймінги, Wi-Fi.
 * Тут міняємо цифри; логіка в інших модулях.
 */

// Збірка без Bluetooth-геймпада:  -DENABLE_GAMEPAD=0  (build_flags у platformio.ini)
#ifndef ENABLE_GAMEPAD
#define ENABLE_GAMEPAD 1
#endif

#include <Arduino.h>

// ============================================================
// ПІНИ
// ============================================================
constexpr uint8_t PIN_LED_BT = 4;
constexpr uint8_t PIN_LED_ERROR = 16;
constexpr uint8_t PIN_LED_STATUS = 17;

constexpr uint8_t PIN_STEP_ARM1 = 13;
constexpr uint8_t PIN_DIR_ARM1 = 26;
constexpr uint8_t PIN_STEP_ARM2 = 23;
constexpr uint8_t PIN_DIR_ARM2 = 14;
constexpr uint8_t PIN_STEP_Z = 18;
constexpr uint8_t PIN_DIR_Z = 19;

constexpr uint8_t PIN_SERVO_ROTATE = 27;
constexpr uint8_t PIN_SERVO_GRIP = 32;

constexpr uint8_t PIN_BUTTON_TEACH = 25;
constexpr uint8_t PIN_BUTTON_MODE = 33;
constexpr uint8_t PIN_ESTOP = 39;

// ============================================================
// МЕХАНІКА
// ============================================================
constexpr float ARM_LENGTH_1_MM = 91.61f;
constexpr float ARM_LENGTH_2_MM = 97.528f;

constexpr float MOTOR_STEPS_REV = 200.0f;
constexpr float MICROSTEPS = 16.0f;

constexpr float GEAR_ARM1 = 4.5f;
constexpr float GEAR_ARM2 = 7.280f;
constexpr float Z_SCREW_LEAD_MM = 2.0f;
constexpr float GEAR_Z = 1.0f;

// Знак осі: розворот напрямку без перепаювання
constexpr int8_t SIGN_ARM1 = 1;
constexpr int8_t SIGN_ARM2 = 1;
constexpr int8_t SIGN_Z = -1;

constexpr float STEPS_PER_DEG_ARM1 = MOTOR_STEPS_REV * MICROSTEPS * GEAR_ARM1 / 360.0f;
constexpr float STEPS_PER_DEG_ARM2 = MOTOR_STEPS_REV * MICROSTEPS * GEAR_ARM2 / 360.0f;
constexpr float STEPS_PER_MM_Z = MOTOR_STEPS_REV * MICROSTEPS * GEAR_Z / Z_SCREW_LEAD_MM;

// ============================================================
// HOME (ручний) і ПРОГРАМНІ ЛІМІТИ
// ============================================================
constexpr float HOME_ARM1_DEG = 115.0f;
constexpr float HOME_ARM2_DEG = -147.0f;
constexpr float HOME_Z_MM = 0.0f;
constexpr float HOME_TOOL_ROTATE_DEG = 90.0f;

constexpr float ARM1_MIN_DEG = -115.0f;
constexpr float ARM1_MAX_DEG = 115.0f;
// ВИПРАВЛЕНО: мінімум A2 тепер НЕ вищий за HOME (-147), інакше при першій
// команді рука сама смикалась на -138.
constexpr float ARM2_MIN_DEG = -150.0f;
constexpr float ARM2_MAX_DEG = 138.0f;
constexpr float Z_MIN_MM = 0.0f;
constexpr float Z_MAX_MM = 300.0f;
constexpr float TOOL_ROTATE_MIN_DEG = 0.0f;
constexpr float TOOL_ROTATE_MAX_DEG = 180.0f;

// ---- СХВАТ: РЕАЛЬНІ кути серви (0..180°) ----
// !!! ПІДІБРАТИ ПІД МЕХАНІКУ !!!
// GRIP_MIN_DEG = повністю відкрито (безпечна межа, не виходить із пазів)
// GRIP_MAX_DEG = повністю закрито.
// Зменшуй/збільшуй по 10°, поки схват не стане рівно в пазах.
constexpr float GRIP_MIN_DEG = 60.0f;
constexpr float GRIP_MAX_DEG = 130.0f;
constexpr float HOME_GRIP_DEG = GRIP_MIN_DEG;

constexpr float MIN_ARM_RADIUS_MM = 45.0f;
constexpr float MAX_ARM_RADIUS_MM = 185.0f;

// ============================================================
// ШВИДКОСТІ ТА ПРИСКОРЕННЯ
// ============================================================
constexpr float ARM1_JOG_SPEED_DEG_S = 45.0f;
constexpr float ARM2_JOG_SPEED_DEG_S = 45.0f;
constexpr float Z_JOG_SPEED_MM_S = 10.0f; // було 3.5
constexpr float TOOL_XY_SPEED_MM_S = 40.0f;

constexpr float AUTO_ARM1_SPEED_DEG_S = 40.0f;
constexpr float AUTO_ARM2_SPEED_DEG_S = 40.0f;
constexpr float AUTO_Z_SPEED_MM_S = 10.0f; // було 3.5

// Швидкість повернення в HOME (кнопка Y). Обмежена ARMx_MAX_STEP_HZ / Z_MAX_STEP_HZ.
constexpr float HOME_ARM1_SPEED_DEG_S = 90.0f; // 90 * 40 кроків/° = 3600 Гц
constexpr float HOME_ARM2_SPEED_DEG_S = 80.0f; // 80 * 64.7 кроків/° = ~5200 Гц
constexpr float HOME_Z_SPEED_MM_S = 12.0f;     // 12 * 1600 кроків/мм = 19200 Гц

constexpr float TOOL_ROTATE_SPEED_DEG_S = 45.0f;
constexpr float GRIP_SPEED_DEG_S = 120.0f;         // швидкість цілі схвата при повному натиску (було 300 — давало ривки)
constexpr float GRIP_RAMP_S = 0.15f;               // час плавного розгону/гальмування команди схвата
constexpr float GRIP_OUTPUT_SLEW_DEG_S = 180.0f;   // макс. швидкість зміни кута, що йде на серву (плавність, у т.ч. при HOME)

constexpr int SERVO_ROTATE_MIN_US = 500;
constexpr int SERVO_ROTATE_MAX_US = 2500;
constexpr int SERVO_GRIP_MIN_US = 500;
constexpr int SERVO_GRIP_MAX_US = 2500;
constexpr float SERVO_ROTATE_FILTER_ALPHA = 0.45f;

constexpr uint32_t ARM1_MAX_STEP_HZ = 12000;
constexpr uint32_t ARM2_MAX_STEP_HZ = 6000;
constexpr uint32_t Z_MAX_STEP_HZ = 20000; // 20000 / 1600 = 12.5 мм/с

constexpr uint32_t ARM1_ACCEL = 14000;
constexpr uint32_t ARM2_ACCEL = 8000;
constexpr uint32_t Z_ACCEL = 20000; // було 6000: швидший розгін/гальмування

// ============================================================
// 3 КОЛОНКИ
// ============================================================
constexpr float COLUMN_X_MM[3] = {-75.0f, 0.0f, 75.0f};
constexpr float COLUMN_Y_MM[3] = {110.0f, 135.0f, 110.0f};
constexpr float COLUMN_Z_TOP_MM = 15.0f;
constexpr float COLUMN_Z_BOTTOM_MM = 120.0f;
constexpr uint32_t COLUMN_DWELL_MS = 350;

// ============================================================
// ЧАС І НАЛАШТУВАННЯ
// ============================================================
constexpr uint32_t CONTROL_PERIOD_MS = 10;
constexpr uint32_t SERVO_PERIOD_MS = 20;
constexpr uint32_t GAMEPAD_TIMEOUT_MS = 10000;
constexpr uint32_t GAMEPAD_ARM_NEUTRAL_MS = 300;
constexpr uint32_t BUTTON_DEBOUNCE_MS = 40;
constexpr uint32_t TEACH_LONG_PRESS_MS = 1800;
constexpr uint32_t DEFAULT_POINT_DWELL_MS = 400;
constexpr uint32_t JOG_WATCHDOG_MS = 1800; // веб шле джог кожні 0.6 с
constexpr uint32_t DIAG_PERIOD_MS = 5000;
constexpr int16_t JOYSTICK_DEADZONE = 90;
constexpr float MAX_CONTROL_DT_S = 0.05f;
constexpr uint8_t MAX_PROGRAM_POINTS = 20;
constexpr bool AUTO_LOOP_PROGRAM = true;

// ---- Wi-Fi ----
constexpr const char *AP_SSID = "RoboArm-WiFi";
constexpr const char *AP_PASS = "12345678";
constexpr uint8_t AP_CHANNEL = 1;
constexpr uint8_t AP_MAX_CLIENTS = 4;
constexpr uint32_t WS_STATE_PERIOD_MS = 100; // як часто сервер штовхає стан у веб-панель
constexpr bool CAPTIVE_PORTAL_POPUP = true; // true = спливашка при підключенні, false = без неї

// ---- Плавний веб-рух (див. motion.cpp: processWebMotion) ----
constexpr float WEB_SMOOTH_S = 0.12f;        // постійна часу гальмування біля цілі
constexpr float WEB_LEAD_TICKS = 3.0f;       // на скільки тактів уперед ставимо ціль драйверу
constexpr float WEB_ROT_SPEED_DEG_S = 90.0f; // швидкість утримуваного повороту кисті
