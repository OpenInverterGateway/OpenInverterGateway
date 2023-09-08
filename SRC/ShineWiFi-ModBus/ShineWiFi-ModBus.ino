/*

Add ESP8266 compiler to arduino IDE
  - In your Arduino IDE, go to File -> Preferences
  - Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into the "Additional Boards Manager URLs"

Used Libs
  - WiFiManager         by tzapu                  https://github.com/tzapu/WiFiManager
  - PubSubClient        by Nick O´Leary           https://github.com/knolleary/pubsubclient
  - DoubleResetDetector by Khai Hoang             https://github.com/khoih-prog/ESP_DoubleResetDetector
  - ModbusMaster        by Doc Walker             https://github.com/4-20ma/ModbusMaster
  - ArduinoJson         by Benoit Blanchon        https://github.com/bblanchon/ArduinoJson
  - Preferences         by vshymanskyy            https://github.com/vshymanskyy/Preferences
  - TeeLog              by Dirk-Willem van Gulik  https://github.com/dirkx/tee-log.git

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

#include "ShineWifi.h"
#include <TLog.h>
#include "Index.h"
#include "Growatt.h"
#include <Preferences.h>
#include <WiFiManager.h>
#include <StreamUtils.h>

#ifdef ESP32
    #include <esp_task_wdt.h>
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
    #include "ShineMqtt.h"
#endif

Preferences prefs;
Growatt Inverter;
bool StartedConfigAfterBoot = false;

#if MQTT_SUPPORTED == 1
    #ifdef MQTTS_ENABLED
        WiFiClientSecure espClient;
    #else
        WiFiClient espClient;
    #endif
    ShineMqtt shineMqtt(espClient, Inverter);
#endif

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
            Log.print(F("x"));
            digitalWrite(LED_RT, !digitalRead(LED_RT)); // toggle red led on WiFi (re)connect
        }

        // todo: use Log
        WiFi.printDiag(Serial);
        Log.print(F("local IP:"));
        Log.println(WiFi.localIP());
        Log.print(F("Hostname: "));
        Log.println(HOSTNAME);

        Log.println(F("WiFi reconnected"));

        digitalWrite(LED_RT, 1);
    }
}

// Connection can fail after sunrise. The stick powers up before the inverter.
// So the detection of the inverter will fail. If no inverter is detected, we have to retry later (s. loop() )
// The detection without running inverter will take several seconds, because the ModBus-Lib has a timeout of 2s 
// for each read access (and we do several of them). The WiFi can crash during this function. Perhaps we can fix 
// this by using the callback function of the ModBus-Lib
void InverterReconnect(void)
{
    // Baudrate will be set here, depending on the version of the stick
    Inverter.begin(Serial);

    if (Inverter.GetWiFiStickType() == ShineWiFi_S)
        Log.println(F("ShineWiFi-S (Serial) found"));
    else if (Inverter.GetWiFiStickType() == ShineWiFi_X)
        Log.println(F("ShineWiFi-X (USB) found"));
    else
        Log.println(F("Error: Unknown Shine Stick"));
}

#if MQTT_SUPPORTED == 1
void loadConfig(MqttConfig* config);
void saveConfig(MqttConfig* config);
void saveParamCallback();
void SetupMqttWifiManagerMenu(MqttConfig &mqttConfig);

void loadConfig(MqttConfig* config)
{
    config->mqttserver = prefs.getString(serverfile, "10.1.2.3");
    config->mqttport = prefs.getString(portfile, "1883");
    config->mqtttopic = prefs.getString(topicfile, "energy/solar");
    config->mqttuser = prefs.getString(userfile, "");
    config->mqttpwd = prefs.getString(secretfile, "");
}

void saveConfig(MqttConfig* config)
{
    prefs.putString(serverfile, config->mqttserver);
    prefs.putString(portfile, config->mqttport);
    prefs.putString(topicfile, config->mqtttopic);
    prefs.putString(userfile, config->mqttuser);
    prefs.putString(secretfile, config->mqttpwd);
}

void saveParamCallback()
{
    Log.println(F("[CALLBACK] saveParamCallback fired"));
    MqttConfig config;

    config.mqttserver = custom_mqtt_server->getValue();
    config.mqttport = custom_mqtt_port->getValue();
    config.mqtttopic = custom_mqtt_topic->getValue();
    config.mqttuser = custom_mqtt_user->getValue();
    config.mqttpwd = custom_mqtt_pwd->getValue();

    saveConfig(&config);

    Serial.println(F("[CALLBACK] saveParamCallback complete"));
}
#endif

#ifdef ENABLE_TELNET_DEBUG
#include <TelnetSerialStream.h>
TelnetSerialStream telnetSerialStream = TelnetSerialStream();
#endif

#ifdef ENABLE_WEB_DEBUG
#include <WebSerialStream.h>
WebSerialStream webSerialStream = WebSerialStream(8080);
#endif

void configureLogging() {
    #ifdef ENABLE_SERIAL_DEBUG
        Serial.begin(115200);
        Log.disableSerial(false);
    #else
        Log.disableSerial(true);
    #endif
    #ifdef ENABLE_TELNET_DEBUG
        Log.addPrintStream(std::make_shared<TelnetSerialStream>(telnetSerialStream));
    #endif
    #ifdef ENABLE_WEB_DEBUG
        Log.addPrintStream(std::make_shared<WebSerialStream>(webSerialStream));
    #endif
}

void setupGPIO() 
{
    pinMode(LED_GN, OUTPUT);
    pinMode(LED_RT, OUTPUT);
    pinMode(LED_BL, OUTPUT);    
}


void setupWifiHost()
{
    WiFi.hostname(HOSTNAME);
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    MDNS.begin(HOSTNAME);
}

void startWdt() 
{
    #ifdef ESP32
    Log.println("Configuring WDT...");
    esp_task_wdt_init(WDT_TIMEOUT, true);
    esp_task_wdt_add(NULL);
    #endif
}

void handleWdtReset(boolean mqttSuccess) 
{
    #if MQTT_SUPPORTED == 1
    if(mqttSuccess) {
        resetWdt();
    } else {
        if(!shineMqtt.mqttEnabled()) {
            resetWdt();
        }
    }
    #else
        resetWdt();
    #endif
}

void resetWdt() 
{
    #ifdef ESP32
    Log.println("WDT reset ...");
    esp_task_wdt_reset();
    #endif
}

void setup()
{
    Log.println("Setup()");

    configureLogging();
    setupGPIO();

    #if ENABLE_DOUBLE_RESET == 1
        drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
    #endif

    #if MQTT_SUPPORTED == 1
        prefs.begin("ShineWifi");
    #endif

    #if ENABLE_DOUBLE_RESET == 1
        if (drd->detectDoubleReset()) {
        Log.println(F("Double reset detected"));
            StartedConfigAfterBoot = true;
        }
    #endif

    setupWifiHost();

    Log.begin();
    startWdt();

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
        Log.println(F("Failed to connect WIFI"));
        ESP.restart();
    }
    else
    {
        digitalWrite(LED_BL, 0);
        //if you get here you have connected to the WiFi
        Log.println(F("WIFI connected...yeey :)"));
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

    httpServer.on("/status", sendJsonSite);
    httpServer.on("/uistatus", sendUiJsonSite);
    httpServer.on("/StartAp", StartConfigAccessPoint);
    httpServer.on("/postCommunicationModbus", SendPostSite);
    httpServer.on("/postCommunicationModbus_p", HTTP_POST, handlePostData);
    httpServer.on("/", MainPage);
    #ifdef ENABLE_WEB_DEBUG
        httpServer.on("/debug", SendDebug);
    #endif

    Inverter.InitProtocol();
    InverterReconnect();
    httpServer.begin();
}

#if MQTT_SUPPORTED == 1
void SetupMqttWifiManagerMenu(MqttConfig &mqttConfig) {
    loadConfig(&mqttConfig);

    custom_mqtt_server = new WiFiManagerParameter("server", "mqtt server", mqttConfig.mqttserver.c_str(), 40);
    custom_mqtt_port = new WiFiManagerParameter("port", "mqtt port", mqttConfig.mqttport.c_str(), 6);
    custom_mqtt_topic = new WiFiManagerParameter("topic", "mqtt topic", mqttConfig.mqtttopic.c_str(), 64);
    custom_mqtt_user = new WiFiManagerParameter("username", "mqtt username", mqttConfig.mqttuser.c_str(), 40);
    custom_mqtt_pwd = new WiFiManagerParameter("password", "mqtt password", mqttConfig.mqttpwd.c_str(), 64);

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
    std::vector<const char*> menu = { "wifi","wifinoscan","update"};
    if(enableCustomParams){
        menu.push_back("param");
    }
    menu.push_back("sep");
    menu.push_back("erase");
    menu.push_back("restart");
    
    wm.setMenu(menu); // custom menu, pass vector
}

void sendJson(ShineJsonDocument&  doc)
{
    httpServer.setContentLength(measureJson(doc));
    httpServer.send(200, "application/json", "");
    WiFiClient client = httpServer.client();
    WriteBufferingStream bufferedWifiClient{client, BUFFER_SIZE};
    serializeJson(doc, bufferedWifiClient);
}

void sendJsonSite(void)
{
    StaticJsonDocument<JSON_DOCUMENT_SIZE> doc;
    Inverter.CreateJson(doc, WiFi.macAddress());

    sendJson(doc);
}

void sendUiJsonSite(void)
{
    StaticJsonDocument<JSON_DOCUMENT_SIZE> doc;
    Inverter.CreateUIJson(doc);

    sendJson(doc);
}

#if MQTT_SUPPORTED == 1
boolean sendMqttJson(void)
{
    StaticJsonDocument<JSON_DOCUMENT_SIZE> doc;

    Inverter.CreateJson(doc, WiFi.macAddress());
    return shineMqtt.mqttPublish(doc);
}
#endif

void StartConfigAccessPoint(void)
{
    String Text;
    Text = "<html><body>Configuration access point started ...<br /><br />Connect to Wifi: \"GrowattConfig\" with your password (default: \"growsolar\") and visit <a href='http://192.168.4.1'>192.168.4.1</a><br />The Stick will automatically go back to normal operation after " + String(CONFIG_PORTAL_MAX_TIME_SECONDS) + " seconds</body></html>";
    httpServer.send(200, "text/html", Text);
    StartedConfigAfterBoot = true;
}

#ifdef ENABLE_WEB_DEBUG
void SendDebug(void) {
    httpServer.sendHeader("Location", "http://" + WiFi.localIP().toString() + ":8080/", true);
    httpServer.send ( 302, "text/plain", "");
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
    char msg[256];
    uint16_t u16Tmp;
    uint32_t u32Tmp;

    if (!httpServer.hasArg(F("reg")) || !httpServer.hasArg(F("val")))
    {
        // If the POST request doesn't have data
        httpServer.send(400, F("text/plain"), F("400: Invalid Request")); // The request is invalid, so send HTTP status 400
        return;
    }
    else
    {
        if (httpServer.arg(F("operation")) == "R")
        {
            if (httpServer.arg(F("registerType")) == "I")
            {
                if (httpServer.arg(F("type")) == "16b")
                {
                    if (Inverter.ReadInputReg(httpServer.arg(F("reg")).toInt(), &u16Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b Input register %ld with value %d"), httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b Input register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadInputReg(httpServer.arg(F("reg")).toInt(), &u32Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b Input register %ld with value %d"), httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b Input register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
            }
            else
            {
                if (httpServer.arg(F("type")) == "16b")
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg(F("reg")).toInt(), &u16Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b Holding register %ld with value %d"), httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg),PSTR("Read 16b Holding register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg(F("reg")).toInt(), &u32Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b Holding register %ld with value %d"), httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b Holding register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
            }
        }
        else
        {
            if (httpServer.arg(F("registerType")) == "H")
            {
                if (httpServer.arg(F("type")) == "16b")
                {
                    if (Inverter.WriteHoldingReg(httpServer.arg(F("reg")).toInt(), httpServer.arg(F("val")).toInt()))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Wrote Holding Register %ld to a value of %ld!"), httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b Holding register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    snprintf_P(msg, sizeof(msg), PSTR("Writing to double (32b) registers not supported"));
                }
            }
            else
            {
                snprintf_P(msg, sizeof(msg), PSTR("It is not possible to write into Input Registers"));
            }
        }
        httpServer.send(200, F("text/plain"), msg);
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

    Log.loop();
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
                Log.println(F("Handle press"));
                StartedConfigAfterBoot = true;
            }
            else
            {
                btnPressed++;
            }
            Log.print(F("Btn pressed"));
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
        Log.println(F("Config after boot started"));
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
                    Log.println(F("ReadData() successful"));
                    u16PacketCnt++;
                    u8RetryCounter = NUM_OF_RETRIES;
                    boolean mqttSuccess = false;

                    #if MQTT_SUPPORTED == 1
                    mqttSuccess = sendMqttJson();
                    #endif
                    handleWdtReset(mqttSuccess);

                    digitalWrite(LED_RT, 0); // clear red led if everything is ok
                    // leave while-loop
                    readoutSucceeded = 1;
                }
                else
                {
                    Log.println(F("ReadData() NOT successful"));
                    if (u8RetryCounter)
                    {
                        u8RetryCounter--;
                    }
                    else
                    {
                        Log.println(F("Retry counter"));
                        #if MQTT_SUPPORTED == 1
                            shineMqtt.mqttPublish(String(F("{\"InverterStatus\": -1 }")));
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
