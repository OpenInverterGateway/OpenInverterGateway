#ifndef _SHINEW_IFI_MQTT_H
#define _SHINEW_IFI_MQTT_H
#include "Config.h"

#if MQTT_SUPPORTED == 1
    #include <Arduino.h>
    #include "ShineWifi.h"
    #include <PubSubClient.h>
    #include <stdbool.h>
    #include "WebDebug.h"

    extern PubSubClient MqttClient;

    typedef struct {
        String mqttserver;
        String mqttport;
        String mqtttopic;
        String mqttuser;
        String mqttpwd ;
    } MqttConfig;

    bool MqttReconnect();
    void MqttSetup(const MqttConfig &config);
    void MqttPublish(const String &JsonString);
    void updateMqttLed();
#endif
#endif
