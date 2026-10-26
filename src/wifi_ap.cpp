#include "wifi_ap.h"
#include <Arduino.h>
#include <WiFi.h>
#include <DNSServer.h>
#include "config.h"

DNSServer dnsServer;
const byte DNS_PORT = 53;

void onWifiEvent(arduino_event_id_t event)
{
    switch (event)
    {
    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
        Serial.printf("[WiFi] клієнт підключився (усього %d)\n", WiFi.softAPgetStationNum());
        break;
    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
        Serial.printf("[WiFi] клієнт відключився (усього %d)\n", WiFi.softAPgetStationNum());
        break;
    default:
        break;
    }
}

void startWifiAp()
{
    // ВАЖЛИВО: Wi-Fi піднімаємо ДО Bluetooth, без setSleep(false) і без max TX power.
    WiFi.persistent(false);
    WiFi.onEvent(onWifiEvent);
    WiFi.mode(WIFI_AP);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    const bool ok = WiFi.softAP(AP_SSID, AP_PASS, AP_CHANNEL, 0, AP_MAX_CLIENTS);
    Serial.printf("Wi-Fi AP '%s' пароль '%s' канал %u: %s  IP %s\n",
                  AP_SSID, AP_PASS, AP_CHANNEL, ok ? "OK" : "ПОМИЛКА",
                  WiFi.softAPIP().toString().c_str());

    dnsServer.start(DNS_PORT, "*", WiFi.softAPIP());
}

void processWifiDns() { dnsServer.processNextRequest(); }
