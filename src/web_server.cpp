#include "web_server.h"
#include <WebServer.h>
#include <WebSocketsServer.h> // links2004/WebSockets
#include "config.h"
#include "state.h"
#include "utils.h"
#include "motion.h"
#include "column_cycle.h"
#include "program.h"
#include "auto_mode.h"
#include "panel.h"
#include "web_ui.h"

WebServer server(80);

// Редірект на панель (captive portal)
void captiveRedirect()
{
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
}

// ---- Канал WebSocket (порт 81): одне постійне з'єднання замість нового TCP на кожен запит ----
WebSocketsServer wsServer(81);
bool wsActive = false; // зараз виконується команда, що прийшла по WebSocket
uint8_t wsNum = 0;
String wsReqId;
String wsQuery;

bool argHas(const char *name)
{
    if (!wsActive) return server.hasArg(name);
    const String key = String(name) + "=";
    int i = 0;
    while (i <= (int)wsQuery.length())
    {
        int e = wsQuery.indexOf('&', i);
        if (e < 0) e = wsQuery.length();
        if (wsQuery.startsWith(key, i)) return true;
        i = e + 1;
    }
    return false;
}

String argGet(const char *name)
{
    if (!wsActive) return server.arg(name);
    const String key = String(name) + "=";
    int i = 0;
    while (i <= (int)wsQuery.length())
    {
        int e = wsQuery.indexOf('&', i);
        if (e < 0) e = wsQuery.length();
        if (wsQuery.startsWith(key, i)) return wsQuery.substring(i + key.length(), e);
        i = e + 1;
    }
    return String();
}

void sendJson(int code, const String &body)
{
    if (wsActive)
    {
        String m = "R|" + wsReqId + "|" + String(code) + "|" + body;
        wsServer.sendTXT(wsNum, m.c_str());
        return;
    }
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json; charset=utf-8", body);
}

// Ручні команди з вебу дозволені лише в MANUAL і без E-STOP
bool webGuard()
{
    if (robot.estopLatched)
    {
        sendJson(403, "{\"error\":\"E-STOP активний\"}");
        return false;
    }
    if (robot.operatingMode != OperatingMode::MANUAL)
    {
        sendJson(409, "{\"error\":\"Зараз режим AUTO — перейдіть у ручний\"}");
        return false;
    }
    return true;
}

void handleRoot()
{
    server.sendHeader("Cache-Control", "no-cache");
    server.send_P(200, "text/html; charset=utf-8", INDEX_HTML);
}

// Конфіг для веб-інтерфейсу: ліміти й геометрія завжди збігаються з прошивкою
void handleConfig()
{
    String j = "{";
    j += "\"a1\":[" + String(ARM1_MIN_DEG, 1) + "," + String(ARM1_MAX_DEG, 1) + "],";
    j += "\"a2\":[" + String(ARM2_MIN_DEG, 1) + "," + String(ARM2_MAX_DEG, 1) + "],";
    j += "\"z\":[" + String(Z_MIN_MM, 1) + "," + String(Z_MAX_MM, 1) + "],";
    j += "\"rot\":[" + String(TOOL_ROTATE_MIN_DEG, 1) + "," + String(TOOL_ROTATE_MAX_DEG, 1) + "],";
    j += "\"g\":[" + String(GRIP_MIN_DEG, 1) + "," + String(GRIP_MAX_DEG, 1) + "],";
    j += "\"l1\":" + String(ARM_LENGTH_1_MM, 3) + ",";
    j += "\"l2\":" + String(ARM_LENGTH_2_MM, 3) + ",";
    j += "\"rmin\":" + String(MIN_ARM_RADIUS_MM, 1) + ",";
    j += "\"rmax\":" + String(MAX_ARM_RADIUS_MM, 1) + ",";
    j += "\"pts\":" + String(MAX_PROGRAM_POINTS) + ",";
    j += "\"cols\":[";
    for (int i = 0; i < 3; i++)
    {
        if (i) j += ",";
        j += "[" + String(COLUMN_X_MM[i], 1) + "," + String(COLUMN_Y_MM[i], 1) + "]";
    }
    j += "]}";
    sendJson(200, j);
}

String buildStateJson()
{
    updateActualRobotPosition();
    const JointPosition &cmd = webFollow ? webTarget : robot.joints;
    String j = "{";
    j += "\"a1\":" + String(cmd.arm1Deg, 2) + ",";
    j += "\"a2\":" + String(cmd.arm2Deg, 2) + ",";
    j += "\"z\":" + String(cmd.zMm, 2) + ",";
    j += "\"rot\":" + String(robot.toolRotateDeg, 1) + ",";
    j += "\"grip\":" + String(robot.gripperDeg, 1) + ",";
    j += "\"act_a1\":" + String(robot.actualJoints.arm1Deg, 2) + ",";
    j += "\"act_a2\":" + String(robot.actualJoints.arm2Deg, 2) + ",";
    j += "\"act_z\":" + String(robot.actualJoints.zMm, 2) + ",";
    j += "\"act_x\":" + String(robot.actualTcp.xMm, 1) + ",";
    j += "\"act_y\":" + String(robot.actualTcp.yMm, 1) + ",";
    j += "\"mode\":\"" + String(robot.operatingMode == OperatingMode::AUTO ? "AUTO" : "MANUAL") + "\",";
    j += "\"auto\":" + String(static_cast<uint8_t>(robot.autoState)) + ",";
    j += "\"estop\":" + String(robot.estopLatched ? "true" : "false") + ",";
    j += "\"running\":" + String(axesRunning() ? "true" : "false") + ",";
    j += "\"gpad\":" + String(robot.controllerConnected ? "true" : "false") + ",";
    j += "\"col_act\":" + String(columnCycleActive ? "true" : "false") + ",";
    j += "\"col_id\":" + String(currentColumnId) + ",";
    j += "\"points\":" + String(programPointCount);
    j += "}";
    return j;
}

void handleState() { sendJson(200, buildStateJson()); }

void handleSet()
{
    if (!webGuard()) return;

    jogContinuousActive = false;
    padHoming = false;
    stopColumnCycle();

    JointPosition t = webFollow ? webTarget : robot.joints;
    if (argHas("l1")) t.arm1Deg = argGet("l1").toFloat();
    if (argHas("l2")) t.arm2Deg = argGet("l2").toFloat();
    if (argHas("z")) t.zMm = argGet("z").toFloat();

    if (argHas("rot"))
        robot.toolRotateDeg = clampFloat(argGet("rot").toFloat(), TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
    if (argHas("g"))
        robot.gripperDeg = clampFloat(argGet("g").toFloat(), GRIP_MIN_DEG, GRIP_MAX_DEG);

    if (argHas("l1") || argHas("l2") || argHas("z"))
        startWebFollow(t);

    sendJson(200, "{\"status\":\"ok\"}");
}

void handleJog()
{
    if (!webGuard()) return;
    if (!argHas("axis"))
    {
        sendJson(400, "{\"error\":\"немає axis\"}");
        return;
    }

    padHoming = false;
    stopColumnCycle();

    const String axis = argGet("axis");
    const float dir = (argHas("dir") && argGet("dir").toFloat() < 0.0f) ? -1.0f : 1.0f;
    const float step = argHas("step") ? argGet("step").toFloat() : 0.0f;

    JointPosition t = webFollow ? webTarget : robot.joints; // база — ціль, тож швидкі кліки складаються

    if (step != 0.0f)
    {
        jogContinuousActive = false;
        if (axis == "z") t.zMm += step;
        else if (axis == "l1") t.arm1Deg += step;
        else if (axis == "l2") t.arm2Deg += step;
        else if (axis == "rot")
        {
            robot.toolRotateDeg = clampFloat(robot.toolRotateDeg + step, TOOL_ROTATE_MIN_DEG, TOOL_ROTATE_MAX_DEG);
            sendJson(200, "{\"status\":\"ok\"}");
            return;
        }
        startWebFollow(t);
        sendJson(200, "{\"status\":\"ok\"}");
        return;
    }

    // Неперервний джог: їдемо до ліміту, поки веб повторює команду
    if (axis == "rot")
    {
        webRotDir = dir; // плавно крутить сама прошивка, поки веб повторює команду
        webRotWatchdogMs = millis() + 500;
        sendJson(200, "{\"status\":\"ok\"}");
        return;
    }

    updateActualRobotPosition();
    const JointPosition base = webFollow ? webTarget : robot.actualJoints;
    bool atLimit = false;
    if (axis == "z")
    {
        atLimit = (dir > 0 && base.zMm >= Z_MAX_MM) || (dir < 0 && base.zMm <= Z_MIN_MM);
        t = base;
        t.zMm = (dir > 0) ? Z_MAX_MM : Z_MIN_MM;
    }
    else if (axis == "l1")
    {
        atLimit = (dir > 0 && base.arm1Deg >= ARM1_MAX_DEG) || (dir < 0 && base.arm1Deg <= ARM1_MIN_DEG);
        t = base;
        t.arm1Deg = (dir > 0) ? ARM1_MAX_DEG : ARM1_MIN_DEG;
    }
    else if (axis == "l2")
    {
        atLimit = (dir > 0 && base.arm2Deg >= ARM2_MAX_DEG) || (dir < 0 && base.arm2Deg <= ARM2_MIN_DEG);
        t = base;
        t.arm2Deg = (dir > 0) ? ARM2_MAX_DEG : ARM2_MIN_DEG;
    }
    else
    {
        sendJson(400, "{\"error\":\"невідома вісь\"}");
        return;
    }

    if (atLimit)
    {
        jogContinuousActive = false;
        sendJson(200, "{\"status\":\"limit\"}");
        return;
    }

    jogContinuousActive = true;
    jogWatchdogMs = millis() + JOG_WATCHDOG_MS;
    startWebFollow(t);
    sendJson(200, "{\"status\":\"ok\"}");
}

void handleJogStop()
{
    if (argHas("axis") && argGet("axis") == "rot")
    {
        webRotDir = 0.0f; // тільки кисть, інші осі не чіпаємо
        sendJson(200, "{\"status\":\"stopped\"}");
        return;
    }
    softStopMotion(); // плавне гальмування замість різкого обриву
    sendJson(200, "{\"status\":\"stopped\"}");
}

// Стоп: зупиняє все. В AUTO — ставить на паузу (можна продовжити), в MANUAL — просто стоп.
void handleStop()
{
    jogContinuousActive = false;
    if (robot.operatingMode == OperatingMode::AUTO)
        pauseAuto("WEB СТОП — AUTO ПАУЗА");
    emergencyStopMotion();
    sendJson(200, "{\"status\":\"stopped\"}");
}

void handleHome()
{
    if (!webGuard()) return;
    jogContinuousActive = false;
    padHoming = false;
    stopColumnCycle();
    const JointPosition home = {HOME_ARM1_DEG, HOME_ARM2_DEG, HOME_Z_MM};
    commandWebJointPosition(home);
    robot.toolRotateDeg = HOME_TOOL_ROTATE_DEG;
    robot.gripperDeg = HOME_GRIP_DEG;
    sendJson(200, "{\"status\":\"homing\"}");
}

void handleColumn()
{
    if (!webGuard()) return;

    uint8_t mode = 0;
    if (argHas("id"))
    {
        const int v = argGet("id").toInt(); // "all" -> 0
        if (v >= 1 && v <= 3) mode = static_cast<uint8_t>(v);
    }
    jogContinuousActive = false;
    padHoming = false;
    startColumnCycle(mode);
    sendJson(200, "{\"status\":\"started\"}");
}

void handleColumnStop()
{
    stopColumnCycle();
    sendJson(200, "{\"status\":\"stopped\"}");
}

void handleTeach()
{
    if (!webGuard()) return;
    teachCurrentPoint();
    sendJson(200, "{\"status\":\"ok\",\"points\":" + String(programPointCount) + "}");
}

void handleClear()
{
    if (!webGuard()) return;
    clearProgram();
    sendJson(200, "{\"status\":\"ok\",\"points\":" + String(programPointCount) + "}");
}

void handleAuto()
{
    if (robot.estopLatched)
    {
        sendJson(403, "{\"error\":\"E-STOP активний\"}");
        return;
    }
    if (robot.operatingMode != OperatingMode::AUTO)
    {
        sendJson(409, "{\"error\":\"Спочатку перейдіть в режим AUTO\"}");
        return;
    }
    toggleAutoRunPause();
    sendJson(200, "{\"status\":\"ok\",\"auto_state\":" + String(static_cast<uint8_t>(robot.autoState)) + "}");
}

void handleMode()
{
    toggleOperatingMode();
    sendJson(200, "{\"status\":\"ok\",\"mode\":\"" +
                      String(robot.operatingMode == OperatingMode::AUTO ? "AUTO" : "MANUAL") + "\"}");
}

// ============================================================
// WebSocket: команди "id|/шлях?аргументи" -> ті самі обробники, що й HTTP
// відповідь: "R|id|код|json", стан (push): "S|json"
// ============================================================
struct WsRoute { const char *path; void (*fn)(); };
static const WsRoute WS_ROUTES[] = {
    {"/config", handleConfig}, {"/state", handleState}, {"/set", handleSet},
    {"/jog", handleJog}, {"/jog_stop", handleJogStop}, {"/stop", handleStop},
    {"/home", handleHome}, {"/teach", handleTeach}, {"/clear", handleClear},
    {"/auto", handleAuto}, {"/mode", handleMode}, {"/column", handleColumn},
    {"/column_stop", handleColumnStop}};

void wsDispatch(uint8_t num, const String &msg)
{
    const int bar = msg.indexOf('|');
    if (bar < 0) return;
    wsNum = num;
    wsReqId = msg.substring(0, bar);
    String path = msg.substring(bar + 1);
    const int q = path.indexOf('?');
    wsQuery = (q >= 0) ? path.substring(q + 1) : String();
    if (q >= 0) path = path.substring(0, q);

    wsActive = true;
    bool found = false;
    for (const WsRoute &r : WS_ROUTES)
    {
        if (path == r.path)
        {
            r.fn();
            found = true;
            break;
        }
    }
    if (!found) sendJson(404, "{\"error\":\"невідомий шлях\"}");
    wsActive = false;
}

void wsEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_CONNECTED:
    {
        Serial.printf("[WS] клієнт %u підключився\n", num);
        const String s = "S|" + buildStateJson();
        wsServer.sendTXT(num, s.c_str());
        break;
    }
    case WStype_DISCONNECTED:
        Serial.printf("[WS] клієнт %u відключився\n", num);
        // зв'язок зник посеред утримуваного джогу — зупиняємось одразу
        if (jogContinuousActive || webRotDir != 0.0f)
        {
            webRotDir = 0.0f;
            softStopMotion();
        }
        break;
    case WStype_TEXT:
        if (length > 0 && length < 200) wsDispatch(num, String(reinterpret_cast<const char *>(payload)));
        break;
    default:
        break;
    }
}

// ============================================================
// Ініціалізація і цикл
// ============================================================
void setupWebServer()
{
    server.on("/", HTTP_GET, handleRoot);
    server.on("/config", HTTP_GET, handleConfig);
    server.on("/state", HTTP_GET, handleState);
    server.on("/set", HTTP_GET, handleSet);
    server.on("/jog", HTTP_GET, handleJog);
    server.on("/jog_stop", HTTP_GET, handleJogStop);
    server.on("/stop", HTTP_GET, handleStop);
    server.on("/home", HTTP_GET, handleHome);
    server.on("/teach", HTTP_GET, handleTeach);
    server.on("/clear", HTTP_GET, handleClear);
    server.on("/auto", HTTP_GET, handleAuto);
    server.on("/mode", HTTP_GET, handleMode);
    server.on("/column", HTTP_GET, handleColumn);
    server.on("/column_stop", HTTP_GET, handleColumnStop);
    // Перевірки "чи є інтернет" від телефонів/ПК.
    // CAPTIVE_PORTAL_POPUP = true  -> редірект на панель: при підключенні САМА вискакує сторінка (як було раніше)
    // CAPTIVE_PORTAL_POPUP = false -> відповідаємо "інтернет є": без спливашки, зате телефон стабільніше тримає Wi-Fi
    static const char IOS_OK[] = "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>";
    if (CAPTIVE_PORTAL_POPUP)
    {
        server.on("/generate_204", HTTP_GET, captiveRedirect);
        server.on("/gen_204", HTTP_GET, captiveRedirect);
        server.on("/hotspot-detect.html", HTTP_GET, captiveRedirect);
        server.on("/library/test/success.html", HTTP_GET, captiveRedirect);
        server.on("/ncsi.txt", HTTP_GET, captiveRedirect);
        server.on("/connecttest.txt", HTTP_GET, captiveRedirect);
        server.on("/redirect", HTTP_GET, captiveRedirect);
        server.on("/canonical.html", HTTP_GET, captiveRedirect);
        server.on("/success.txt", HTTP_GET, captiveRedirect);
        server.onNotFound(captiveRedirect);
    }
    else
    {
        server.on("/generate_204", HTTP_GET, []() { server.send(204, "text/plain", ""); });
        server.on("/gen_204", HTTP_GET, []() { server.send(204, "text/plain", ""); });
        server.on("/hotspot-detect.html", HTTP_GET, []() { server.send(200, "text/html", IOS_OK); });
        server.on("/library/test/success.html", HTTP_GET, []() { server.send(200, "text/html", IOS_OK); });
        server.on("/ncsi.txt", HTTP_GET, []() { server.send(200, "text/plain", "Microsoft NCSI"); });
        server.on("/connecttest.txt", HTTP_GET, []() { server.send(200, "text/plain", "Microsoft Connect Test"); });
        server.onNotFound([]() { server.send(404, "text/plain", ""); });
    }
    server.on("/favicon.ico", HTTP_GET, []() { server.send(204, "text/plain", ""); });

    server.begin();
    wsServer.begin();
    wsServer.onEvent(wsEvent);
    wsServer.enableHeartbeat(10000, 3000, 2); // мертвий клієнт звільняє слот за ~16 с
    Serial.println("Веб-панель: http://192.168.4.1  (WebSocket :81)");
}

void processWebServer()
{
    wsServer.loop();
    server.handleClient();
}

// Штовхаємо стан у веб-панель (замість опитування)
void pushWebState(uint32_t now)
{
    static uint32_t lastWsState = 0;
    if (now - lastWsState >= WS_STATE_PERIOD_MS && wsServer.connectedClients() > 0)
    {
        lastWsState = now;
        const String s = "S|" + buildStateJson();
        wsServer.broadcastTXT(s.c_str());
    }
}