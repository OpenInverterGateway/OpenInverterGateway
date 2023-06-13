#pragma once

#include "Config.h"

#if MQTT_SUPPORTED == 1
#include <Arduino.h>
#include "ShineWifi.h"
#include "Growatt.h"
#include <PubSubClient.h>
#include <stdbool.h>

typedef struct {
  String mqttserver;
  String mqttport;
  String mqtttopic;
  String mqttuser;
  String mqttpwd;
} MqttConfig;

class ShineMqtt {
 public:
  ShineMqtt(WiFiClient& wc, Growatt& inverter);
  void mqttSetup(const MqttConfig& config);
  bool mqttReconnect();
  void mqttPublish(const String& JsonString);
  void onMqttMessage(char* topic, byte* payload, unsigned int length);
  void updateMqttLed();
  void loop();

 private:
  WiFiClient& wifiClient;
  long previousConnectTryMillis = 0;
  MqttConfig mqttconfig;
  PubSubClient mqttclient;
  Growatt& inverter;
  static String getId();
};
#endif
