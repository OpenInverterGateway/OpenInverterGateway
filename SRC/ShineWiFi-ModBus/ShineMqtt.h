#pragma once

#include "Config.h"
#include "StickConfig.h"

#if MQTT_SUPPORTED == 1
#include <Arduino.h>
#include "ShineWifi.h"
#include <PubSubClient.h>
#include <stdbool.h>

class ShineMqtt {
 public:
  ShineMqtt(WiFiClient& wc) : wifiClient(wc), mqttclient(wifiClient){};
  void mqttSetup(const StickConfig& config);
  bool mqttReconnect();
  void mqttPublish(const String& JsonString);
  void updateMqttLed();
  void loop();

 private:
  WiFiClient& wifiClient;
  long previousConnectTryMillis = 0;
  StickConfig mqttconfig;
  PubSubClient mqttclient;
  static String getId();
};
#endif
