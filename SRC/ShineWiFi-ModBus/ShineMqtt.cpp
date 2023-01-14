#include "ShineMqtt.h"
#if MQTT_SUPPORTED == 1
#include "PubSubClient.h"

void ShineMqtt::mqttSetup(const MqttConfig& config) {
  this->mqttconfig = config;

  uint16_t intPort = config.mqttport.toInt();
  if (intPort == 0) intPort = 1883;

#if ENABLE_DEBUG_OUTPUT == 1
  Serial.print(F("MqttServer: "));
  Serial.println(this->mqttconfig.mqttserver);
  Serial.print(F("MqttPort: "));
  Serial.println(intPort);
  Serial.print(F("MqttTopic: "));
  Serial.println(this->mqttconfig.mqtttopic);
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
#if ENABLE_DEBUG_OUTPUT == 1
    Serial.print("MqttServer: ");
    Serial.println(this->mqttconfig.mqttserver);
    Serial.print("MqttUser: ");
    Serial.println(this->mqttconfig.mqttuser);
    Serial.print("MqttTopic: ");
    Serial.println(this->mqttconfig.mqtttopic);
    Serial.print("Attempting MQTT connection...");
#endif

    // Run only once every 5 seconds
    this->previousConnectTryMillis = millis();
    // Attempt to connect with last will
    if (this->mqttclient.connect(getId().c_str(),
                                 this->mqttconfig.mqttuser.c_str(),
                                 this->mqttconfig.mqttpwd.c_str(),
                                 this->mqttconfig.mqtttopic.c_str(), 1, 1,
                                 "{\"InverterStatus\": -1 }")) {
#if ENABLE_DEBUG_OUTPUT == 1
      Serial.println("connected");
      return true;
#endif
    } else {
#if ENABLE_DEBUG_OUTPUT == 1
      Serial.print("failed, rc=");
      Serial.print(this->mqttclient.state());
      Serial.println(" try again in 5 seconds");
#endif
      WEB_DEBUG_PRINT("MQTT Connect failed")
      previousConnectTryMillis = millis();
    }
  }
  return false;
}

void ShineMqtt::mqttPublish(const String& JsonString) {
  if (this->mqttclient.connected())
    this->mqttclient.publish(this->mqttconfig.mqtttopic.c_str(),
                             JsonString.c_str(), true);
}

void ShineMqtt::updateMqttLed() {
  if (!this->mqttclient.connected())
    digitalWrite(LED_RT, 1);
  else
    digitalWrite(LED_RT, 0);
}

void ShineMqtt::loop() { this->mqttclient.loop(); }
#endif
