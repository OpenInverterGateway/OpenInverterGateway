#ifndef GROWATT_SHINEWIFI_S_SHINEWIFI_H
#define GROWATT_SHINEWIFI_S_SHINEWIFI_H
#ifdef ESP8266
    #include <ESP8266WiFi.h>
    #include <ESP8266WebServer.h>
#elif ESP32
    #include <WiFi.h>
    #include <WebServer.h>
#endif

extern WiFiClient espClient;
#endif
