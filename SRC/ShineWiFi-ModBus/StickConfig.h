#pragma once
#include <Arduino.h>

typedef struct {
  String mqttserver;
  String mqttport;
  String mqtttopic;
  String mqttuser;
  String mqttpwd;
} StickConfig;