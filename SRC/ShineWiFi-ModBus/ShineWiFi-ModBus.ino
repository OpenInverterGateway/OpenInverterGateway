/*

Add ESP8266 compiler to arduino IDE
  - In your Arduino IDE, go to File -> Preferences
  - Enter http://arduino.esp8266.com/stable/package_esp8266com_index.json into the "Additional Boards Manager URLs"

Used Libs
  - WiFiManager         by tzapu           https://github.com/tzapu/WiFiManager
  - PubSubClient        by Nick O´Leary    https://github.com/knolleary/pubsubclient
  - DoubleResetDetector by Khai Hoang      https://github.com/khoih-prog/ESP_DoubleResetDetector
  - ModbusMaster        by Doc Walker      https://github.com/knolleary/pubsubclient
  - ArduinoJson         by Benoit Blanchon https://github.com/bblanchon/ArduinoJson

To install the used libraries, use the embedded library manager (Sketch -> Include Library -> Manage Libraries),
or download them from github (Sketch -> Include Library -> Add .ZIP Library)

Thanks to Jethro Kairys
https://github.com/jkairys/growatt-esp8266

File -> "Show verbose output during:" "compilation".
This will show the path to the binary during compilation
e.g. C:\Users\<username>\AppData\Local\Temp\arduino_build_533155


*/
// ---------------------------------------------------------------
// User configuration area start
// ---------------------------------------------------------------

// Configuration file that contains the individual configuration and secret data (wifi password...)
// Rename the Config.h.example from the repo to Config.h and add all your config data to it
// The Config.h has been added to the .gitignore, so that your secrets will be kept
#include "Config.h"

#ifdef ESP8266
    #include <ESP8266HTTPUpdateServer.h>
#elif ESP32
    #include <ESPHTTPUpdateServer.h>
#endif

#ifdef ENABLE_DOUBLE_RESET
    #define ESP_DRD_USE_LITTLEFS true
    #define ESP_DRD_USE_EEPROM false
    #define DRD_TIMEOUT 10
    #define DRD_ADDRESS 0
    #include <ESP_DoubleResetDetector.h>
DoubleResetDetector* drd;
#endif

#if ENABLE_WEB_DEBUG == 1
char acWebDebug[1024] = "";
uint16_t u16WebMsgNo = 0;
    #define WEB_DEBUG_PRINT(s)                                                    \
        {                                                                         \
            if ((strlen(acWebDebug) + strlen(s) + 50) < sizeof(acWebDebug))       \
                sprintf(acWebDebug, "%s#%i: %s\n", acWebDebug, u16WebMsgNo++, s); \
        }
#else
    #define WEB_DEBUG_PRINT(s) ;
#endif

// ---------------------------------------------------------------
// User configuration area end
// ---------------------------------------------------------------

#include "LittleFS.h"

#ifdef ESP8266
    #include <ESP8266WebServer.h>
    #include <ESP8266WiFi.h>
#elif ESP32
    #include <WebServer.h>
    #include <WiFi.h>
#endif

#if MQTT_SUPPORTED == 1
    #include <PubSubClient.h>
#endif

#include "Growatt.h"
bool StartedConfigAfterBoot = false;
#define CONFIG_PORTAL_MAX_TIME_SECONDS 300
#include "index.h"
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

#if PINGER_SUPPORTED == 1
    #include <Pinger.h>
    #include <PingerResponse.h>
#endif

#define LED_GN 0  // GPIO0
#define LED_RT 2  // GPIO2
#define LED_BL 16 // GPIO16

#define FORMAT_LITTLEFS_IF_FAILED true

byte btnPressed = 0;

#define NUM_OF_RETRIES 5
char u8RetryCounter = NUM_OF_RETRIES;

const char* update_path = "/firmware";
uint16_t u16PacketCnt = 0;
#if PINGER_SUPPORTED == 1
Pinger pinger;
#endif

WiFiClient espClient;
#if MQTT_SUPPORTED == 1
PubSubClient MqttClient(espClient);

long previousConnectTryMillis = 0;
#endif
Growatt Inverter;
#ifdef ESP8266
ESP8266WebServer httpServer(80);
#elif ESP32
WebServer httpServer(80);
#endif

#ifdef ESP8266
ESP8266HTTPUpdateServer httpUpdater;
#elif ESP32
ESPHTTPUpdateServer httpUpdater;
#endif
WiFiManager wm;
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

String mqttserver = "";
String mqttport = "";
String mqtttopic = "";
String mqttuser = "";
String mqttpwd = "";

char JsonString[MQTT_MAX_PACKET_SIZE] = "{\"InverterStatus\": -1 }";

// -------------------------------------------------------
// Check the WiFi status and reconnect if necessary
// -------------------------------------------------------
void WiFi_Reconnect()
{
    uint16_t cnt = 0;

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

// -------------------------------------------------------
// Check the Mqtt status and reconnect if necessary
// -------------------------------------------------------
#if MQTT_SUPPORTED == 1
bool MqttReconnect()
{
    if (mqttserver.length() == 0)
    {
        // No server configured
        return false;
    }

    if (WiFi.status() != WL_CONNECTED)
        return false;

    if (MqttClient.connected())
        return true;

    if (millis() - previousConnectTryMillis >= (5000))
    {
    #if ENABLE_DEBUG_OUTPUT == 1
        Serial.print("MqttServer: ");
        Serial.println(mqttserver);
        Serial.print("MqttUser: ");
        Serial.println(mqttuser);
        Serial.print("MqttTopic: ");
        Serial.println(mqtttopic);
        Serial.print("Attempting MQTT connection...");
    #endif

        // Run only once every 5 seconds
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
#endif

String load_from_file(const char* file_name, String defaultvalue)
{
    String result = "";

    File this_file = LittleFS.open(file_name, "r");
    if (!this_file)
    { // failed to open the file, return defaultvalue
        return defaultvalue;
    }

    while (this_file.available())
    {
        result += (char)this_file.read();
    }

    this_file.close();
    return result;
}

bool write_to_file(const char* file_name, String contents)
{
    File this_file = LittleFS.open(file_name, "w");
    if (!this_file)
    { // failed to open the file, return false
        return false;
    }

    int bytesWritten = this_file.print(contents);

    if (bytesWritten == 0)
    { // write failed
        return false;
    }

    this_file.close();
    return true;
}

void saveParamCallback()
{
    Serial.println("[CALLBACK] saveParamCallback fired");
    mqttserver = custom_mqtt_server->getValue();
    write_to_file(serverfile, mqttserver);

    mqttport = custom_mqtt_port->getValue();
    write_to_file(portfile, mqttport);

    mqtttopic = custom_mqtt_topic->getValue();
    write_to_file(topicfile, mqtttopic);

    mqttuser = custom_mqtt_user->getValue();
    write_to_file(userfile, mqttuser);

    mqttpwd = custom_mqtt_pwd->getValue();
    write_to_file(secretfile, mqttpwd);

    if (StartedConfigAfterBoot)
    {
        ESP.restart();
    }
}

String getId()
{
#ifdef ESP8266
    uint64_t id = ESP.getChipId();
#elif ESP32
    uint64_t id = ESP.getEfuseMac();
#endif

    return String("Growatt" + id);
}

void setup()
{
#if ENABLE_DEBUG_OUTPUT == 1
    Serial.begin(115200);
    Serial.println(F("Setup()"));
#endif
    WEB_DEBUG_PRINT("Setup()");

#ifdef ENABLE_DOUBLE_RESET
    drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
#endif

    pinMode(LED_GN, OUTPUT);
    pinMode(LED_RT, OUTPUT);
    pinMode(LED_BL, OUTPUT);

#ifdef ESP8266
    LittleFS.begin();
#elif ESP32
    LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED);
#endif

#if MQTT_SUPPORTED == 1
    mqttserver = load_from_file(serverfile, "10.1.2.3");
    mqttport = load_from_file(portfile, "1883");
    mqtttopic = load_from_file(topicfile, "energy/solar");
    mqttuser = load_from_file(userfile, "");
    mqttpwd = load_from_file(secretfile, "");
#endif

#ifdef ENABLE_DOUBLE_RESET
    if (drd->detectDoubleReset())
    {
    #if ENABLE_DEBUG_OUTPUT == 1
        Serial.println(F("Double reset detected"));
    #endif
        StartedConfigAfterBoot = true;
    }
#endif

    WiFi.hostname(HOSTNAME);
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

#if MQTT_SUPPORTED == 1
                         // make sure the packet size is set correctly in the library
    MqttClient.setBufferSize(MQTT_MAX_PACKET_SIZE);

    custom_mqtt_server = new WiFiManagerParameter("server", "mqtt server", mqttserver.c_str(), 40);
    custom_mqtt_port = new WiFiManagerParameter("port", "mqtt port", mqttport.c_str(), 6);
    custom_mqtt_topic = new WiFiManagerParameter("topic", "mqtt topic", mqtttopic.c_str(), 64);
    custom_mqtt_user = new WiFiManagerParameter("username", "mqtt username", mqttuser.c_str(), 40);
    custom_mqtt_pwd = new WiFiManagerParameter("password", "mqtt password", mqttpwd.c_str(), 40);

    wm.addParameter(custom_mqtt_server);
    wm.addParameter(custom_mqtt_port);
    wm.addParameter(custom_mqtt_topic);
    wm.addParameter(custom_mqtt_user);
    wm.addParameter(custom_mqtt_pwd);
    wm.setSaveParamsCallback(saveParamCallback);

    std::vector<const char*> menu = {"wifi", "wifinoscan", "param", "sep", "erase", "restart"};
    wm.setMenu(menu); // custom menu, pass vector
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
        // if you get here you have connected to the WiFi
        Serial.println(F("connected...yeey :)"));
#endif
    }

    while (WiFi.status() != WL_CONNECTED)
    {
        WiFi_Reconnect();
    }

#if MQTT_SUPPORTED == 1
    uint16_t port = mqttport.toInt();
    if (port == 0)
        port = 1883;
    #if ENABLE_DEBUG_OUTPUT == 1
    Serial.print(F("MqttServer: "));
    Serial.println(mqttserver);
    Serial.print(F("MqttPort: "));
    Serial.println(port);
    Serial.print(F("MqttTopic: "));
    Serial.println(mqtttopic);
    #endif
    MqttClient.setServer(mqttserver.c_str(), port);
#endif

    httpServer.on("/status", SendJsonSite);
    httpServer.on("/uistatus", SendUiJsonSite);
    httpServer.on("/postCommunicationModbus", SendPostSite);
    httpServer.on("/postCommunicationModbus_p", HTTP_POST, handlePostData);
    httpServer.on("/", MainPage);
#if ENABLE_WEB_DEBUG == 1
    httpServer.on("/debug", SendDebug);
#endif

    Inverter.InitProtocol();
    InverterReconnect();

    httpUpdater.setup(&httpServer, update_path, UPDATE_USER, UPDATE_PASSWORD);
    httpServer.begin();
}

void SendJsonSite(void)
{
    JsonString[0] = '\0';
    Inverter.CreateJson(JsonString, WiFi.macAddress().c_str());
    httpServer.send(200, "application/json", JsonString);
}

void SendUiJsonSite(void)
{
    JsonString[0] = '\0';
    Inverter.CreateUIJson(JsonString);
    httpServer.send(200, "application/json", JsonString);
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
    httpServer.send(200, "text/html",
                    "<form action=\"/postCommunicationModbus_p\" method=\"POST\">"
                    "<input type=\"text\" name=\"reg\" placeholder=\"Register ID\"></br>"
                    "<input type=\"text\" name=\"val\" placeholder=\"Input Value (16bit only!)\"></br>"
                    "<select name=\"type\"><option value=\"16b\" selected>16b</option><option value=\"32b\">32b</option></select></br>"
                    "<select name=\"operation\"><option value=\"R\" selected>Read</option><option value=\"W\">Write</option></select></br>"
                    "<select name=\"registerType\"><option value=\"I\" selected>Input Register</option><option value=\"H\">Holding Register</option></select></br>"
                    "<input type=\"submit\" value=\"Go\">"
                    "</form>");
}

void handlePostData()
{
    char* msg;
    uint16_t u16Tmp;
    uint32_t u32Tmp;

    msg = JsonString;
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
                        sprintf(msg, "Read 16b Input register %d with value %d", httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 16b Input register %d impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadInputReg(httpServer.arg("reg").toInt(), &u32Tmp))
                    {
                        sprintf(msg, "Read 32b Input register %d with value %d", httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 32b Input register %d impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
            }
            else
            {
                if (httpServer.arg("type") == "16b")
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg("reg").toInt(), &u16Tmp))
                    {
                        sprintf(msg, "Read 16b Holding register %d with value %d", httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 16b Holding register %d impossible - not connected?", httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg("reg").toInt(), &u32Tmp))
                    {
                        sprintf(msg, "Read 32b Holding register %d with value %d", httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        sprintf(msg, "Read 32b Holding register %d impossible - not connected?", httpServer.arg("reg").toInt());
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
                        sprintf(msg, "Wrote Holding Register %d to a value of %d!", httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
                    }
                    else
                    {
                        sprintf(msg, "Read 16b Holding register %d impossible - not connected?", httpServer.arg("reg").toInt());
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
#ifdef ENABLE_DOUBLE_RESET
    drd->loop();
#endif

    long now = millis();
    char readoutSucceeded;

    if ((now - ButtonTimer) > BUTTON_TIMER)
    {
        ButtonTimer = now;

        if (AP_BUTTON_PRESSED)
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
    if (MqttReconnect())
    {
        MqttClient.loop();
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
                    JsonString[0] = '\0';
                    Inverter.CreateJson(JsonString, WiFi.macAddress().c_str());

#if MQTT_SUPPORTED == 1
                    if (MqttClient.connected())
                        MqttClient.publish(mqtttopic.c_str(), JsonString, true);
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
                        sprintf(JsonString, "{\"InverterStatus\": -1 }");
#if MQTT_SUPPORTED == 1
                        if (MqttClient.connected())
                            MqttClient.publish(mqtttopic.c_str(), JsonString, true);
#endif
                        digitalWrite(LED_RT, 1); // set red led in case of error
                    }
                }
            }
            u8RetryCounter = NUM_OF_RETRIES;
        }

#if MQTT_SUPPORTED == 1
        if (!MqttClient.connected())
            digitalWrite(LED_RT, 1);
        else
            digitalWrite(LED_RT, 0);
#endif

#if PINGER_SUPPORTED == 1
        // frequently check if gateway is reachable
        if (pinger.Ping(GATEWAY_IP) == false)
            WiFi.disconnect();
#endif

        RefreshTimer = now;
    }
}
