#include "ShineMqtt.h"
#if MQTT_SUPPORTED == 1
#include <TLog.h>
#include "PubSubClient.h"

#define MQTT_LWT_OFFLINE "offline"
#define MQTT_LWT_ONLINE "online"

void ShineMqtt::mqttSetup(const MqttConfig& config) {
  this->mqttconfig = config;

  uint16_t intPort = config.mqttport.toInt();
  if (intPort == 0) intPort = 1883;

#if ENABLE_DEBUG_OUTPUT == 1
  Log.print(F("MqttServer: "));
  Log.println(this->mqttconfig.mqttserver);
  Log.print(F("MqttPort: "));
  Log.println(intPort);
  Log.print(F("MqttTopic: "));
  Log.println(this->mqttconfig.mqtttopic);
  Log.print(F("MqttLWT: "));
  Log.println(this->mqttconfig.mqttlwt);
#endif

  // make sure the packet size is set correctly in the library
  this->mqttclient.setBufferSize(MQTT_MAX_PACKET_SIZE);
  this->mqttclient.setServer(this->mqttconfig.mqttserver.c_str(), intPort);
}

String ShineMqtt::getId() {
#ifdef ESP8266
  uint64_t id = ESP.getChipId();
#elif ESP32
  uint64_t id = ESP.getEfuseMac();
#endif
  return "Growatt" + (id & 0xffffffff);
}

// -------------------------------------------------------
// Check the Mqtt status and reconnect if necessary
// -------------------------------------------------------
bool ShineMqtt::mqttReconnect() {
  if (this->mqttconfig.mqttserver.length() == 0) {
    // No server configured
    return false;
  }

  if (WiFi.status() != WL_CONNECTED) return false;

  if (this->mqttclient.connected()) return true;

  if (millis() - this->previousConnectTryMillis >= (5000)) {
#if ENABLE_DEBUG_OUTPUT == 1
    Log.print(F("MqttServer: "));
    Log.println(this->mqttconfig.mqttserver);
    Log.print(F("MqttUser: "));
    Log.println(this->mqttconfig.mqttuser);
    Log.print(F("MqttTopic: "));
    Log.println(this->mqttconfig.mqtttopic);
    Log.print(F("MqttLWT: "));
    Log.println(this->mqttconfig.mqttlwt);
    Log.print(F("Attempting MQTT connection..."));
#endif

    // Run only once every 5 seconds
    this->previousConnectTryMillis = millis();
    // Attempt to connect with last will
    if (this->mqttclient.connect(getId().c_str(),
                                 this->mqttconfig.mqttuser.c_str(),
                                 this->mqttconfig.mqttpwd.c_str(),
                                 this->mqttconfig.mqttlwt.c_str(), 1, 1,
                                 MQTT_LWT_OFFLINE)) {
#if ENABLE_DEBUG_OUTPUT == 1
      Log.println("connected");
#endif
      return true;
    } else {
      Log.print("failed, rc=");
      Log.print(this->mqttclient.state());
      Log.println(" try again in 5 seconds");
      Log.println("MQTT Connect failed");
      previousConnectTryMillis = millis();
    }
  }
  return false;
}

void ShineMqtt::mqttPublish(const String& JsonString) {
  Log.print("publish MQTT message... ");
  if (this->mqttclient.connected()) {
    bool res = this->mqttclient.publish(this->mqttconfig.mqtttopic.c_str(),
                                        JsonString.c_str(), true);
    Log.println(res ? "succeed" : "failed");
  } else
    Log.println("not connected");
}

void ShineMqtt::updateMqttLed() {
  if (!this->mqttclient.connected()) {
    digitalWrite(LED_RT, 1);
  } else {
    digitalWrite(LED_RT, 0);
    this->mqttclient.publish(this->mqttconfig.mqttlwt.c_str(), MQTT_LWT_ONLINE, true);
  }
}

void ShineMqtt::loop() { this->mqttclient.loop(); }
#endif
