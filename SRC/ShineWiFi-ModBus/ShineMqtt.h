#ifndef _SHINE_SHINE_MQTT_H
#define _SHINE_SHINE_MQTT_H
#include "Config.h"

#include <Arduino.h>
String getChipId();

#if MQTT_SUPPORTED == 1
#include "ShineWifi.h"
#include <PubSubClient.h>
#include <stdbool.h>
#include "WebDebug.h"

typedef struct {
  String mqttserver;
  String mqttport;
  String mqtttopic;
  String mqttuser;
  String mqttpwd;
} MqttConfig;

class ShineMqtt {
 public:
  ShineMqtt(WiFiClient& wc) : wifiClient(wc), mqttclient(wifiClient){};
  void mqttSetup(const MqttConfig& config);
  bool mqttReconnect();
  void mqttPublish(const String& JsonString);
  void updateMqttLed();
  void loop();

 private:
  WiFiClient& wifiClient;
  long previousConnectTryMillis = 0;
  MqttConfig mqttconfig;
  PubSubClient mqttclient;
};
#endif
#endif
