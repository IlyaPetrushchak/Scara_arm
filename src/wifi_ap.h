#pragma once
/* wifi_ap.h — Wi-Fi точка доступу + DNS (captive portal). */

void startWifiAp();   // Wi-Fi піднімаємо ДО Bluetooth
void processWifiDns(); // DNS-запити (у loop)
