/*

Add ESP8266 compiler to arduino IDE
  - In your Arduino IDE, go to File -> Preferences
  - Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into the "Additional Boards Manager URLs"

Used Libs
  - WiFiManager         by tzapu           https://github.com/tzapu/WiFiManager
  - PubSubClient        by Nick O´Leary    https://github.com/knolleary/pubsubclient
  - DoubleResetDetector by Khai Hoang      https://github.com/khoih-prog/ESP_DoubleResetDetector
  - ModbusMaster        by Doc Walker      https://github.com/4-20ma/ModbusMaster
  - ArduinoJson         by Benoit Blanchon https://github.com/bblanchon/ArduinoJson

To install the used libraries, use the embedded library manager (Sketch -> Include Library -> Manage Libraries),
or download them from github (Sketch -> Include Library -> Add .ZIP Library)

Thanks to Jethro Kairys
https://github.com/jkairys/growatt-esp8266

File -> "Show verbose output during:" "compilation".
This will show the path to the binary during compilation
e.g. C:\Users\<username>\AppData\Local\Temp\arduino_build_533155
*/

#include "Config.h"
#ifndef _SHINE_CONFIG_H_
#error Please rename Config.h.example to Config.h
#endif

#include "WebDebug.h"
#include "ShineWifi.h"
#include <WiFiManager.h>
#include "Index.h"
#include "Growatt.h"

#if UPDATE_SUPPORTED == 1
    #ifdef ESP8266
        #include <ESP8266HTTPUpdateServer.h>
    #elif ESP32
        #include <ESPHTTPUpdateServer.h>
    #endif
#endif

#if PINGER_SUPPORTED == 1
    #include <Pinger.h>
    #include <PingerResponse.h>
#endif

#if ENABLE_DOUBLE_RESET == 1
    #define ESP_DRD_USE_LITTLEFS    true
    #define ESP_DRD_USE_EEPROM      false
    #define DRD_TIMEOUT             10
    #define DRD_ADDRESS             0
    #include <ESP_DoubleResetDetector.h>
    DoubleResetDetector* drd;
#endif

#if MQTT_SUPPORTED == 1
    #include "LittleFS.h"
    #include "ShineMqtt.h"
#endif

#if MQTT_SUPPORTED == 1
    #ifdef MQTTS_ENABLED
        WiFiClientSecure espClient;
    #else
        WiFiClient espClient;
    #endif
    ShineMqtt shineMqtt(espClient);
#endif

Growatt Inverter;
bool StartedConfigAfterBoot = false;

#ifdef AP_BUTTON_PRESSED
byte btnPressed = 0;
#endif

#define NUM_OF_RETRIES 5
char u8RetryCounter = NUM_OF_RETRIES;

uint16_t u16PacketCnt = 0;
#if PINGER_SUPPORTED == 1
    Pinger pinger;
#endif

#ifdef ESP8266
    ESP8266WebServer httpServer(80);
#elif ESP32
    WebServer httpServer(80);
#endif

#if UPDATE_SUPPORTED == 1
    const char* update_path = "/firmware";
    
    #ifdef ESP8266
        ESP8266HTTPUpdateServer httpUpdater;
    #elif ESP32
        ESPHTTPUpdateServer httpUpdater;
    #endif
#endif

WiFiManager wm;
#if MQTT_SUPPORTED == 1
    WiFiManagerParameter* custom_mqtt_server = NULL;
    WiFiManagerParameter* custom_mqtt_port = NULL;
    WiFiManagerParameter* custom_mqtt_topic = NULL;
    WiFiManagerParameter* custom_mqtt_user = NULL;
    WiFiManagerParameter* custom_mqtt_pwd = NULL;

    const static char* serverfile = "/mqtts";
    const static char* portfile = "/mqttp";
    const static char* topicfile = "/mqttt";
    const static char* userfile = "/mqttu";
    const static char* secretfile = "/mqttw";
#endif

#define CONFIG_PORTAL_MAX_TIME_SECONDS 300

char JSONChars[MQTT_MAX_PACKET_SIZE] = "{\"InverterStatus\": -1 }";

// -------------------------------------------------------
// Check the WiFi status and reconnect if necessary
// -------------------------------------------------------
void WiFi_Reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(LED_GN, 0);

        wm.autoConnect();

        while (WiFi.status() != WL_CONNECTED)
        {
            delay(200);
            #if ENABLE_DEBUG_OUTPUT == 1
                Serial.print("x");
            #endif
            digitalWrite(LED_RT, !digitalRead(LED_RT)); // toggle red led on WiFi (re)connect
        }

        #if ENABLE_DEBUG_OUTPUT == 1
            Serial.println("");
            WiFi.printDiag(Serial);
            Serial.print("local IP:");
            Serial.println(WiFi.localIP());
            Serial.print("Hostname: ");
            Serial.println(HOSTNAME);
        #endif

        WEB_DEBUG_PRINT("WiFi reconnected")

        digitalWrite(LED_RT, 1);
    }
}

// Conection can fail after sunrise. The stick powers up before the inverter.
// So the detection of the inverter will fail. If no inverter is detected, we have to retry later (s. loop() )
// The detection without running inverter will take several seconds, because the ModBus-Lib has a timeout of 2s 
// for each read access (and we do several of them). The WiFi can crash during this function. Perhaps we can fix 
// this by using the callback function of the ModBus-Lib
void InverterReconnect(void)
{
    // Baudrate will be set here, depending on the version of the stick
    Inverter.begin(Serial);

    #if ENABLE_WEB_DEBUG == 1
        if (Inverter.GetWiFiStickType() == ShineWiFi_S)
            WEB_DEBUG_PRINT("ShineWiFi-S (Serial) found")
        else if (Inverter.GetWiFiStickType() == ShineWiFi_X)
            WEB_DEBUG_PRINT("ShineWiFi-X (USB) found")
        else
            WEB_DEBUG_PRINT("Error: Unknown Shine Stick")
    #endif
}

#if MQTT_SUPPORTED == 1
String load_from_file(const char* file_name, String defaultvalue);
bool write_to_file(const char* file_name, String contents);
void loadConfig(MqttConfig* config);
void saveConfig(MqttConfig* config);
void saveParamCallback();
void SetupMqttWifiManagerMenu(MqttConfig &mqttConfig);

String load_from_file(const char* file_name, String defaultvalue) {
    String result = "";

    File this_file = LittleFS.open(file_name, "r");
    if (!this_file) { // failed to open the file, return defaultvalue
        return defaultvalue;
    }

    while (this_file.available()) {
        result += (char)this_file.read();
    }

    this_file.close();
    return result;
}

bool write_to_file(const char* file_name, String contents) {
    File this_file = LittleFS.open(file_name, "w");
    if (!this_file) { // failed to open the file, return false
        return false;
    }

    int bytesWritten = this_file.print(contents);

    if (bytesWritten == 0) { // write failed
        return false;
    }

    this_file.close();
    return true;
}

void loadConfig(MqttConfig* config)
{
    config->mqttserver = load_from_file(serverfile, "10.1.2.3");
    config->mqttport = load_from_file(portfile, "1883");
    config->mqtttopic = load_from_file(topicfile, "energy/solar");
    config->mqttuser = load_from_file(userfile, "");
    config->mqttpwd = load_from_file(secretfile, "");
}

void saveConfig(MqttConfig* config)
{
    write_to_file(serverfile, config->mqttserver);
    write_to_file(portfile, config->mqttport);
    write_to_file(topicfile, config->mqtttopic);
    write_to_file(userfile, config->mqttuser);
    write_to_file(secretfile, config->mqttpwd);
}

void saveParamCallback()
{
    Serial.println("[CALLBACK] saveParamCallback fired");
    MqttConfig config;

    config.mqttserver = custom_mqtt_server->getValue();
    config.mqttport = custom_mqtt_port->getValue();
    config.mqtttopic = custom_mqtt_topic->getValue();
    config.mqttuser = custom_mqtt_user->getValue();
    config.mqttpwd = custom_mqtt_pwd->getValue();

    saveConfig(&config);

    ESP.restart();
}
#endif

void setup()
{
    #if ENABLE_DEBUG_OUTPUT == 1
        Serial.begin(115200);
        Serial.println(F("Setup()"));
    #endif
    WEB_DEBUG_PRINT("Setup()");

    #if ENABLE_DOUBLE_RESET == 1
        drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
    #endif

    pinMode(LED_GN, OUTPUT);
    pinMode(LED_RT, OUTPUT);
    pinMode(LED_BL, OUTPUT);

    #if MQTT_SUPPORTED == 1
    #ifdef ESP8266
    LittleFS.begin();
    #elif ESP32
    LittleFS.begin(true);
    #endif
    #endif

    #if ENABLE_DOUBLE_RESET == 1
    if (drd->detectDoubleReset()) {
        #if ENABLE_DEBUG_OUTPUT == 1
            Serial.println(F("Double reset detected"));
        #endif
        StartedConfigAfterBoot = true;
    }
    #endif

    WiFi.hostname(HOSTNAME);
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

    #if MQTT_SUPPORTED == 1
        MqttConfig mqttConfig;
        SetupMqttWifiManagerMenu(mqttConfig);
    #endif

    digitalWrite(LED_BL, 1);
    // Set a timeout so the ESP doesn't hang waiting to be configured, for instance after a power failure
    wm.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);
    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ("GrowattConfig")
    bool res = wm.autoConnect("GrowattConfig", APPassword); // password protected wificonfig ap

    if (!res)
    {
        #if ENABLE_DEBUG_OUTPUT == 1
            Serial.println(F("Failed to connect"));
        #endif
        ESP.restart();
    }
    else
    {
        digitalWrite(LED_BL, 0);
        #if ENABLE_DEBUG_OUTPUT == 1
            //if you get here you have connected to the WiFi
            Serial.println(F("WIFI connected...yeey :)"));
        #endif
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi_Reconnect();
    }

    #if MQTT_SUPPORTED == 1
        #ifdef MQTTS_ENABLED
            espClient.setCACert(MQTTS_BROKER_CA_CERT);
        #endif
        shineMqtt.mqttSetup(mqttConfig);
    #else
        setupMenu(false);
    #endif

    httpServer.on("/status", SendJsonSite);
    httpServer.on("/uistatus", SendUiJsonSite);
    httpServer.on("/StartAp", StartConfigAccessPoint);
    httpServer.on("/postCommunicationModbus", SendPostSite);
    httpServer.on("/postCommunicationModbus_p", HTTP_POST, handlePostData);
    httpServer.on("/", MainPage);
    #if ENABLE_WEB_DEBUG == 1
        httpServer.on("/debug", SendDebug);
    #endif

    Inverter.InitProtocol();
    InverterReconnect();
    #if UPDATE_SUPPORTED == 1
        httpUpdater.setup(&httpServer, update_path, UPDATE_USER, UPDATE_PASSWORD);
    #endif
    httpServer.begin();
}

#if MQTT_SUPPORTED == 1
void SetupMqttWifiManagerMenu(MqttConfig &mqttConfig) {
    loadConfig(&mqttConfig);

    custom_mqtt_server = new WiFiManagerParameter("server", "mqtt server", mqttConfig.mqttserver.c_str(), 40);
    custom_mqtt_port = new WiFiManagerParameter("port", "mqtt port", mqttConfig.mqttport.c_str(), 6);
    custom_mqtt_topic = new WiFiManagerParameter("topic", "mqtt topic", mqttConfig.mqtttopic.c_str(), 64);
    custom_mqtt_user = new WiFiManagerParameter("username", "mqtt username", mqttConfig.mqttuser.c_str(), 40);
    custom_mqtt_pwd = new WiFiManagerParameter("password", "mqtt password", mqttConfig.mqttpwd.c_str(), 40);

    wm.addParameter(custom_mqtt_server);
    wm.addParameter(custom_mqtt_port);
    wm.addParameter(custom_mqtt_topic);
    wm.addParameter(custom_mqtt_user);
    wm.addParameter(custom_mqtt_pwd);
    wm.setSaveParamsCallback(saveParamCallback);

    setupMenu(true);
}
#endif

/**
 * @brief create custom wifimanager menu entries
 * 
 * @param enableCustomParams enable custom params aka. mqtt settings
 */
void setupMenu(bool enableCustomParams){
    std::vector<const char*> menu = { "wifi","wifinoscan"};
    if(enableCustomParams){
        menu.push_back("param");
    }
    menu.push_back("sep");
    menu.push_back("erase");
    menu.push_back("restart");
    
    wm.setMenu(menu); // custom menu, pass vector
}


void SendJsonSite(void)
{
    JSONChars[0] = '\0';
    Inverter.CreateJson(JSONChars, WiFi.macAddress().c_str());
    httpServer.send(200, "application/json", JSONChars);
}

void SendUiJsonSite(void)
{
    JSONChars[0] = '\0';
    Inverter.CreateUIJson(JSONChars);
    httpServer.send(200, "application/json", JSONChars);
}

void StartConfigAccessPoint(void)
{
    String Text;
    Text = "Configuration access point started ...\r\nConnect to Wifi: \"GrowattConfig\" with your password (default: \"growsolar\") and visit 192.168.4.1\r\nThe Stick will automatically go back to normal operation after " + String(CONFIG_PORTAL_MAX_TIME_SECONDS) + " seconds";
    httpServer.send(200, "text/plain", Text);
    StartedConfigAfterBoot = true;
}

#if ENABLE_WEB_DEBUG == 1
void SendDebug(void)
{
    httpServer.send(200, "text/plain", acWebDebug);
}
#endif

void MainPage(void)
{
    httpServer.send(200, "text/html", MAIN_page);
}

void SendPostSite(void)
{
    httpServer.send(200, "text/html", SendPostSite_page);
}

void handlePostData()
{
    char* msg;
    uint16_t u16Tmp;
    uint32_t u32Tmp;

    msg = JSONChars;
    msg[0] = 0;

    if (!httpServer.hasArg("reg") || !httpServer.hasArg("val"))
    {
        // If the POST request doesn't have data
        httpServer.send(400, "text/plain", "400: Invalid Request"); // The request is invalid, so send HTTP status 400
        return;
    }
    else
    {
        if (httpServer.arg("operation") == "R")
        {
            if (httpServer.arg("registerType") == "I")
            {
                if (httpServer.arg("type") == "16b")
                {
                    if (Inverter.ReadInputReg(httpServer.arg("reg").toInt(), &u16Tmp))
                    {
                        sprintf(msg, "Read 16b Input register %ld with value %d", httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 16b Input register %ld impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadInputReg(httpServer.arg("reg").toInt(), &u32Tmp))
                    {
                        sprintf(msg, "Read 32b Input register %ld with value %d", httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 32b Input register %ld impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
            }
            else
            {
                if (httpServer.arg("type") == "16b")
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg("reg").toInt(), &u16Tmp))
                    {
                        sprintf(msg, "Read 16b Holding register %ld with value %d", httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 16b Holding register %ld impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg("reg").toInt(), &u32Tmp))
                    {
                        sprintf(msg, "Read 32b Holding register %ld with value %d", httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 32b Holding register %ld impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
            }
        }
        else
        {
            if (httpServer.arg("registerType") == "H")
            {
                if (httpServer.arg("type") == "16b")
                {
                    if (Inverter.WriteHoldingReg(httpServer.arg("reg").toInt(), httpServer.arg("val").toInt()))
                    {
                        sprintf(msg, "Wrote Holding Register %ld to a value of %ld!", httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
                    }
                    else
                    {
                        sprintf(msg, "Read 16b Holding register %ld impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    sprintf(msg, "Writing to double (32b) registers not supported");
                }
            }
            else
            {
                sprintf(msg, "It is not possible to write into Input Registers");
            }
        }
        httpServer.send(200, "text/plain", msg);
        return;
    }
}

// -------------------------------------------------------
// Main loop
// -------------------------------------------------------
long ButtonTimer = 0;
long LEDTimer = 0;
long RefreshTimer = 0;
long WifiRetryTimer = 0;

void loop()
{
    #if ENABLE_DOUBLE_RESET
        drd->loop();
    #endif

    long now = millis();
    char readoutSucceeded;

#ifdef AP_BUTTON_PRESSED
    if ((now - ButtonTimer) > BUTTON_TIMER)
    {
        ButtonTimer = now;

        if( AP_BUTTON_PRESSED )
        {
            if (btnPressed > 5)
            {
                #if ENABLE_DEBUG_OUTPUT == 1
                    Serial.println("Handle press");
                #endif
                StartedConfigAfterBoot = true;
            }
            else
            {
                btnPressed++;
            }
            #if ENABLE_DEBUG_OUTPUT == 1
                Serial.print("Btn pressed");
            #endif
        }
        else
        {
            btnPressed = 0;
        }
    }
#endif

    if (StartedConfigAfterBoot == true)
    {
        digitalWrite(LED_BL, 1);
        httpServer.stop();
        #if ENABLE_DEBUG_OUTPUT == 1
            Serial.println("Config after boot started");
        #endif
        wm.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);
        wm.startConfigPortal("GrowattConfig", APPassword);
        digitalWrite(LED_BL, 0);
        delay(3000);
        ESP.restart();
    }

    WiFi_Reconnect();

    #if MQTT_SUPPORTED == 1
        if (shineMqtt.mqttReconnect())
        {
            shineMqtt.loop();
        }
    #endif

    httpServer.handleClient();

    // Toggle green LED with 1 Hz (alive)
    // ------------------------------------------------------------
    if ((now - LEDTimer) > LED_TIMER)
    {
        if (WiFi.status() == WL_CONNECTED)
            digitalWrite(LED_GN, !digitalRead(LED_GN));
        else
            digitalWrite(LED_GN, 0);

        LEDTimer = now;
    }

    // InverterReconnect() takes a long time --> wifi will crash
    // Do it only every two minutes
    if ((now - WifiRetryTimer) > WIFI_RETRY_TIMER)
    {
        if (Inverter.GetWiFiStickType() == Undef_stick)
            InverterReconnect();
        WifiRetryTimer = now;
    }

    // Read Inverter every REFRESH_TIMER ms [defined in config.h]
    // ------------------------------------------------------------
    if ((now - RefreshTimer) > REFRESH_TIMER)
    {
        if ((WiFi.status() == WL_CONNECTED) && (Inverter.GetWiFiStickType()))
        {
            readoutSucceeded = 0;
            while ((u8RetryCounter) && !(readoutSucceeded))
            {
                #if SIMULATE_INVERTER == 1
                if (1) // do it always
                #else
                if (Inverter.ReadData()) // get new data from inverter
                #endif
                {
                    WEB_DEBUG_PRINT("ReadData() successful")
                    u16PacketCnt++;
                    u8RetryCounter = NUM_OF_RETRIES;

                    // Create JSON string
                    JSONChars[0] = '\0';
                    Inverter.CreateJson(JSONChars, WiFi.macAddress().c_str());

                    #if MQTT_SUPPORTED == 1
                    shineMqtt.mqttPublish(JSONChars);
                    #endif

                    digitalWrite(LED_RT, 0); // clear red led if everything is ok
                    // leave while-loop
                    readoutSucceeded = 1;
                }
                else
                {
                    WEB_DEBUG_PRINT("ReadData() NOT successful")
                    if (u8RetryCounter)
                    {
                        u8RetryCounter--;
                    }
                    else
                    {
                        WEB_DEBUG_PRINT("Retry counter\n")
                        sprintf(JSONChars, "{\"InverterStatus\": -1 }");
                        #if MQTT_SUPPORTED == 1
                            shineMqtt.mqttPublish(JSONChars);
                        #endif
                        digitalWrite(LED_RT, 1); // set red led in case of error
                    }
                }
            }
            u8RetryCounter = NUM_OF_RETRIES;
        }

        #if MQTT_SUPPORTED == 1
        shineMqtt.updateMqttLed();
        #endif

        #if PINGER_SUPPORTED == 1
            //frequently check if gateway is reachable
            if (pinger.Ping(GATEWAY_IP) == false)
                WiFi.disconnect();
        #endif

        RefreshTimer = now;
    }
}
