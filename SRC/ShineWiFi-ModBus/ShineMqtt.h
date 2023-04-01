#ifndef _SHINE_SHINE_MQTT_H
#define _SHINE_SHINE_MQTT_H
#include "Config.h"

#if MQTT_SUPPORTED == 1
#include <Arduino.h>
#include "ShineWifi.h"
#include <PubSubClient.h>
#include <stdbool.h>

typedef struct {
  String mqttserver;
  String mqttport;
  String mqtttopic;
#ifdef ENABLE_MQTT_DEBUG
  String mqttdebugtopic;
#endif
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
  static String getId();
};
#endif
#endif
