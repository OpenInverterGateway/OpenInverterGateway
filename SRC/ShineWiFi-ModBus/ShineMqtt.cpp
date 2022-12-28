#include "ShineMqtt.h"
#if MQTT_SUPPORTED == 1

#include "PubSubClient.h"
String mqttserver = "";
String mqttport = "";
String mqtttopic = "";
String mqttuser = "";
String mqttpwd = "";

PubSubClient MqttClient(espClient);

long previousConnectTryMillis = 0;


void MqttSetup(const MqttConfig &config)
{
    mqttserver = config.mqttserver;
    mqttport = config.mqttport;
    mqtttopic = config.mqtttopic;
    mqttuser = config.mqttuser;
    mqttpwd = config.mqttpwd;

    uint16_t intPort = mqttport.toInt();
    if (intPort == 0)
        intPort = 1883;

    #if ENABLE_DEBUG_OUTPUT == 1
        Serial.print(F("MqttServer: "));
        Serial.println(mqttserver);
        Serial.print(F("MqttPort: "));
        Serial.println(intPort);
        Serial.print(F("MqttTopic: "));
        Serial.println(mqtttopic);
    #endif

    // make sure the packet size is set correctly in the library
    MqttClient.setBufferSize(MQTT_MAX_PACKET_SIZE);
    MqttClient.setServer(mqttserver.c_str(), intPort);
}

String getId()
{
    #ifdef ESP8266
        uint64_t id = ESP.getChipId();
    #elif ESP32
        uint64_t id = ESP.getEfuseMac();
    #endif

    return String("Growatt"+id);
}


// -------------------------------------------------------
// Check the Mqtt status and reconnect if necessary
// -------------------------------------------------------
bool MqttReconnect()
{
    if (mqttserver.length() == 0)
    {
        //No server configured
        return false;
    }

    if (WiFi.status() != WL_CONNECTED)
        return false;

    if (MqttClient.connected())
        return true;

    if (millis() - previousConnectTryMillis >= (5000))
    {
        #if ENABLE_DEBUG_OUTPUT == 1
            Serial.print("MqttServer: "); Serial.println(mqttserver);
            Serial.print("MqttUser: "); Serial.println(mqttuser);
            Serial.print("MqttTopic: "); Serial.println(mqtttopic);
            Serial.print("Attempting MQTT connection...");
        #endif

        //Run only once every 5 seconds
        previousConnectTryMillis = millis();
        // Attempt to connect with last will
        if (MqttClient.connect(getId().c_str(), mqttuser.c_str(), mqttpwd.c_str(), mqtttopic.c_str(), 1, 1, "{\"InverterStatus\": -1 }"))
        {
            #if ENABLE_DEBUG_OUTPUT == 1
                Serial.println("connected");
                return true;
            #endif
        }
        else
        {
            #if ENABLE_DEBUG_OUTPUT == 1
                Serial.print("failed, rc=");
                Serial.print(MqttClient.state());
                Serial.println(" try again in 5 seconds");
            #endif
            WEB_DEBUG_PRINT("MQTT Connect failed")
            previousConnectTryMillis = millis();
        }
    }
    return false;
}

void MqttPublish(const String& JsonString)
{
    if (MqttClient.connected())
        MqttClient.publish(mqtttopic.c_str(), JsonString.c_str(), true);
}

void updateMqttLed() {
    if (!MqttClient.connected())
        digitalWrite(LED_RT, 1);
    else
        digitalWrite(LED_RT, 0);
}

#endif
