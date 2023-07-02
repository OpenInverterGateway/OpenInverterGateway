#pragma once
#include <Arduino.h>

typedef struct {
  String mqtt_server;
  String mqtt_port;
  String mqtt_topic;
  String mqtt_user;
  String mqtt_pwd;
  String update_user;
  String update_pwd;
} StickConfig;