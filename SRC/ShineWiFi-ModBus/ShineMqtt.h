#ifndef _SHINEW_IFI_MQTT_H
#define _SHINEW_IFI_MQTT_H
#include "Config.h"

#if MQTT_SUPPORTED == 1
    #include <Arduino.h>
    #include "ShineWifi.h"
    #include <PubSubClient.h>
    #include <stdbool.h>
    #include "WebDebug.h"

typedef struct {
    String mqttserver;
    String mqttport;
    String mqtttopic;
    String mqttuser;
    String mqttpwd ;
} MqttConfig;

class ShineMqtt {
    public:
        ShineMqtt(WiFiClient& wc): wifiClient(wc), mqttclient(wifiClient) {};
        void mqttSetup(const MqttConfig &config);
        bool mqttReconnect();
        void mqttPublish(const String &JsonString);
        void updateMqttLed();
        void loop();
    private:
        WiFiClient& wifiClient;
        long previousConnectTryMillis = 0;
        MqttConfig mqttconfig;
        PubSubClient mqttclient;
        static String getId();
};
#endif
#endif
