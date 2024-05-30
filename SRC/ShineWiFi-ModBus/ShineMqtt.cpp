#include "ShineMqtt.h"
#include "Growatt.h"
#if MQTT_SUPPORTED == 1
#include <TLog.h>
#include <StreamUtils.h>
#include "PubSubClient.h"

ShineMqtt::ShineMqtt(WiFiClient& wc, Growatt& inverter)
    : wifiClient(wc), mqttclient(wifiClient), inverter(inverter) {}

void ShineMqtt::mqttSetup(const MqttConfig& config) {
  this->mqttconfig = config;

  uint16_t intPort = this->mqttconfig.port.toInt();
  if (intPort == 0) intPort = 1883;

  Log.print(F("MqttServer: "));
  Log.println(this->mqttconfig.server);
  Log.print(F("MqttPort: "));
  Log.println(intPort);
  Log.print(F("MqttTopic: "));
  Log.println(this->mqttconfig.topic);

  this->mqttclient.setServer(this->mqttconfig.server.c_str(), intPort);
  this->mqttclient.setCallback(
      [this](char* topic, byte* payload, unsigned int length) {
        this->onMqttMessage(topic, payload, length);
      });
}

String ShineMqtt::getId() {
#ifdef ESP8266
  uint64_t id = ESP.getChipId();
#elif ESP32
  uint64_t id = ESP.getEfuseMac();
#endif
  return HOSTNAME + String(id & 0xffffffff);
}

boolean ShineMqtt::mqttEnabled() {
  return !this->mqttconfig.server.isEmpty();
}

// -------------------------------------------------------
// Check the Mqtt status and reconnect if necessary
// -------------------------------------------------------
bool ShineMqtt::mqttReconnect() {
  if (!this->mqttEnabled()) {
    // No server configured
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) return false;

  if (this->mqttclient.connected()) return true;

  if (millis() - this->previousConnectTryMillis >= (5000)) {
    Log.print(F("MqttServer: "));
    Log.println(this->mqttconfig.server.c_str());
    Log.print(F("MqttUser: "));
    Log.println(this->mqttconfig.user.c_str());
    Log.print(F("MqttTopic: "));
    Log.println(this->mqttconfig.topic.c_str());
    Log.print(F("Attempting MQTT connection..."));

    // Run only once every 5 seconds
    this->previousConnectTryMillis = millis();
    // Attempt to connect with last will
    if (this->mqttclient.connect(getId().c_str(),
                                 this->mqttconfig.user.c_str(),
                                 this->mqttconfig.pwd.c_str(),
                                 this->mqttconfig.topic.c_str(), 1, 1,
                                 "{\"InverterStatus\": -1 }")) {
      Log.println(F("connected"));

      String commandTopic = this->mqttconfig.topic + "/command/#";
      if (this->mqttclient.subscribe(commandTopic.c_str(), 1)) {
        Log.println("Subscribed to " + commandTopic);
      } else {
        Log.println("Failed to subscribe to " + commandTopic);
      }
      return true;
    } else {
      Log.print(F("failed, rc="));
      Log.print(this->mqttclient.state());
      Log.println(F(" try again in 5 seconds"));
      Log.println(F("MQTT Connect failed"));
      previousConnectTryMillis = millis();
    }
  }
  return false;
}

boolean ShineMqtt::mqttPublish(const String& jsonString) {
  if (jsonString.length() >= BUFFER_SIZE) {
    Log.println(F("MQTT message to long for buffer"));

    return false;
  }

  Log.print(F("publish MQTT message... "));
  if (this->mqttclient.connected()) {
    bool res = this->mqttclient.publish(this->mqttconfig.topic.c_str(),
                                        jsonString.c_str(), true);
    Log.println(res ? "succeed" : "failed");

    return res;
  } else {
    Log.println(F("not connected"));

    return false;
  }
}

boolean ShineMqtt::mqttPublish(JsonDocument& doc, String topic) {
  Log.print(F("publish MQTT message... "));

  if (topic.isEmpty()) {
    topic = this->mqttconfig.topic;
  }

  if (this->mqttclient.connected()) {
    bool res =
        this->mqttclient.beginPublish(topic.c_str(), measureJson(doc), true);
    BufferingPrint bufferedClient(this->mqttclient, BUFFER_SIZE);
    serializeJson(doc, this->mqttclient);
    bufferedClient.flush();
    this->mqttclient.endPublish();

    Log.println(res ? "succeed" : "failed");

    return res;
  } else {
    Log.println(F("not connected"));

    return false;
  }
}

void ShineMqtt::onMqttMessage(char* topic, byte* payload, unsigned int length) {
  StaticJsonDocument<1024> req;
  StaticJsonDocument<1024> res;
  String strTopic(topic);

  Log.print(F("MQTT message arrived ["));
  Log.print(strTopic);
  Log.print(F("] "));

  String command = strTopic.substring(
      String(this->mqttconfig.topic + "/command/").length());
  if (command.isEmpty()) {
    return;
  }

  this->inverter.HandleCommand(command, payload, length, req, res);
  mqttPublish(res, this->mqttconfig.topic + "/result");
}

void ShineMqtt::updateMqttLed() {
  if (!this->mqttclient.connected())
    digitalWrite(LED_RT, 1);
  else
    digitalWrite(LED_RT, 0);
}

void ShineMqtt::loop() { this->mqttclient.loop(); }
#endif
