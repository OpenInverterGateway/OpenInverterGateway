#include "ShineMqtt.h"
#if MQTT_SUPPORTED == 1
#include <TLog.h>
#include "PubSubClient.h"

void ShineMqtt::mqttSetup(const StickConfig& config) {
  this->mqttconfig = config;

  uint16_t intPort = config.mqttport.toInt();
  if (intPort == 0) intPort = 1883;

  Log.print(F("MqttServer: "));
  Log.println(this->mqttconfig.mqttserver);
  Log.print(F("MqttPort: "));
  Log.println(intPort);
  Log.print(F("MqttTopic: "));
  Log.println(this->mqttconfig.mqtttopic);

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
  return "Growatt" + String(id & 0xffffffff);
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
    Log.print("MqttServer: ");
    Log.println(this->mqttconfig.mqttserver.c_str());
    Log.print("MqttUser: ");
    Log.println(this->mqttconfig.mqttuser.c_str());
    Log.print("MqttTopic: ");
    Log.println(this->mqttconfig.mqtttopic.c_str());
    Log.print("Attempting MQTT connection...");

    // Run only once every 5 seconds
    this->previousConnectTryMillis = millis();
    // Attempt to connect with last will
    if (this->mqttclient.connect(getId().c_str(),
                                 this->mqttconfig.mqttuser.c_str(),
                                 this->mqttconfig.mqttpwd.c_str(),
                                 this->mqttconfig.mqtttopic.c_str(), 1, 1,
                                 "{\"InverterStatus\": -1 }")) {
      Log.println("connected");
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
  if (!this->mqttclient.connected())
    digitalWrite(LED_RT, 1);
  else
    digitalWrite(LED_RT, 0);
}

void ShineMqtt::loop() { this->mqttclient.loop(); }
#endif
