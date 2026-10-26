#pragma once
/* web_server.h — HTTP (порт 80) + WebSocket (порт 81) для веб-панелі; captive portal. */
#include <Arduino.h>

void setupWebServer();             // маршрути, captive portal, старт серверів
void processWebServer();           // обробка запитів (у loop)
void pushWebState(uint32_t nowMs); // штовхає стан у панель по WebSocket (у loop)
