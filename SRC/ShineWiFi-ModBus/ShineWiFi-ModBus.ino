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

#if OTA_SUPPORTED == 1
    #include <ArduinoOTA.h>
#endif

#if defined(DEFAULT_NTP_SERVER) && defined(DEFAULT_TZ_INFO)
    #include <time.h>
    extern "C" uint8_t sntp_getreachability(uint8_t);
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
boolean readoutSucceeded = false;

uint16_t u16PacketCnt = 0;
#if PINGER_SUPPORTED == 1
    Pinger pinger;
#endif

#ifdef ESP8266
    ESP8266WebServer httpServer(80);
#elif ESP32
    WebServer httpServer(80);
#endif

struct {
    WiFiManagerParameter* hostname = NULL;
    WiFiManagerParameter* static_ip = NULL;
    WiFiManagerParameter* static_netmask = NULL;
    WiFiManagerParameter* static_gateway = NULL;
    WiFiManagerParameter* static_dns = NULL;
#if MQTT_SUPPORTED == 1
    WiFiManagerParameter* mqtt_server = NULL;
    WiFiManagerParameter* mqtt_port = NULL;
    WiFiManagerParameter* mqtt_topic = NULL;
    WiFiManagerParameter* mqtt_user = NULL;
    WiFiManagerParameter* mqtt_pwd = NULL;
#endif
} customWMParams;

static const struct {
    const char* hostname = "/hostname";
    const char* static_ip = "/staticip";
    const char* static_netmask = "/staticnetmask";
    const char* static_gateway = "/staticgateway";
    const char* static_dns = "/staticdns";
#if MQTT_SUPPORTED == 1
    const char* mqtt_server = "/mqtts";
    const char* mqtt_port = "/mqttp";
    const char* mqtt_topic = "/mqttt";
    const char* mqtt_user = "/mqttu";
    const char* mqtt_pwd = "/mqttw";
#endif
    const char* force_ap = "/forceap";
} ConfigFiles;

struct {
  String hostname;
  String static_ip;
  String static_netmask;
  String static_gateway;
  String static_dns;
#if MQTT_SUPPORTED == 1
  MqttConfig mqtt;
#endif
  bool force_ap;
} Config;

#define CONFIG_PORTAL_MAX_TIME_SECONDS 300

// -------------------------------------------------------
// Set the red led in case of error
// -------------------------------------------------------
void updateRedLed() {
    uint8_t state = 0;
    if (!readoutSucceeded) {
        state = 1;
    }
    if (Inverter.GetWiFiStickType() == Undef_stick) {
        state = 1;
    }
    #if MQTT_SUPPORTED == 1
      if (shineMqtt.mqttEnabled() && !shineMqtt.mqttConnected()) {
        state = 1;
      }
    #endif
    digitalWrite(LED_RT, state);
}

// -------------------------------------------------------
// Check the WiFi status and reconnect if necessary
// -------------------------------------------------------
void WiFi_Reconnect()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        digitalWrite(LED_GN, 0);

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
        Log.println(Config.hostname);

        Log.println(F("WiFi reconnected"));

        updateRedLed();
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

void loadConfig();
void saveConfig();
void saveParamCallback();
void setupWifiManagerConfigMenu(WiFiManager& wm);

void loadConfig()
{
    Config.hostname = prefs.getString(ConfigFiles.hostname, DEFAULT_HOSTNAME);
    Config.static_ip = prefs.getString(ConfigFiles.static_ip, "");
    Config.static_netmask = prefs.getString(ConfigFiles.static_netmask, "");
    Config.static_gateway = prefs.getString(ConfigFiles.static_gateway, "");
    Config.static_dns = prefs.getString(ConfigFiles.static_dns, "");
#if MQTT_SUPPORTED == 1
    Config.mqtt.server = prefs.getString(ConfigFiles.mqtt_server, "10.1.2.3");
    Config.mqtt.port = prefs.getString(ConfigFiles.mqtt_port, "1883");
    Config.mqtt.topic = prefs.getString(ConfigFiles.mqtt_topic, "energy/solar");
    Config.mqtt.user = prefs.getString(ConfigFiles.mqtt_user, "");
    Config.mqtt.pwd = prefs.getString(ConfigFiles.mqtt_pwd, "");
#endif
}

void saveConfig()
{
    prefs.putString(ConfigFiles.hostname, Config.hostname);
    prefs.putString(ConfigFiles.static_ip, Config.static_ip);
    prefs.putString(ConfigFiles.static_netmask, Config.static_netmask);
    prefs.putString(ConfigFiles.static_gateway, Config.static_gateway);
    prefs.putString(ConfigFiles.static_dns, Config.static_dns);
#if MQTT_SUPPORTED == 1
    prefs.putString(ConfigFiles.mqtt_server, Config.mqtt.server);
    prefs.putString(ConfigFiles.mqtt_port, Config.mqtt.port);
    prefs.putString(ConfigFiles.mqtt_topic, Config.mqtt.topic);
    prefs.putString(ConfigFiles.mqtt_user, Config.mqtt.user);
    prefs.putString(ConfigFiles.mqtt_pwd, Config.mqtt.pwd);
#endif
}

void saveParamCallback()
{
    Log.println(F("[CALLBACK] saveParamCallback fired"));

    Config.hostname = customWMParams.hostname->getValue();
    if (Config.hostname.isEmpty()) {
        Config.hostname = DEFAULT_HOSTNAME;
    }
    Config.static_ip = customWMParams.static_ip->getValue();
    Config.static_netmask = customWMParams.static_netmask->getValue();
    Config.static_gateway = customWMParams.static_gateway->getValue();
    Config.static_dns = customWMParams.static_dns->getValue();
#if MQTT_SUPPORTED == 1
    Config.mqtt.server = customWMParams.mqtt_server->getValue();
    Config.mqtt.port = customWMParams.mqtt_port->getValue();
    Config.mqtt.topic = customWMParams.mqtt_topic->getValue();
    Config.mqtt.user = customWMParams.mqtt_user->getValue();
    Config.mqtt.pwd = customWMParams.mqtt_pwd->getValue();
#endif

    saveConfig();

    Serial.println(F("[CALLBACK] saveParamCallback complete"));
}

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
    WiFi.hostname(Config.hostname);
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
    #if OTA_SUPPORTED == 0
        MDNS.begin(Config.hostname);
    #endif
    Log.print(F("setupWifiHost: hostname "));
    Log.println(Config.hostname);
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
    WiFiManager wm;

    Log.println("Setup()");

    configureLogging();
    setupGPIO();

    #if ENABLE_DOUBLE_RESET == 1
        drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
    #endif

    prefs.begin("ShineWifi");

    loadConfig();
    setupWifiHost();

    #if OTA_SUPPORTED == 1
        #if !defined(OTA_PASSWORD)
            #error "Please define an OTA_PASSWORD in Config.h"
        #endif
        ArduinoOTA.setPassword(OTA_PASSWORD);
        ArduinoOTA.setHostname(Config.hostname.c_str());
        ArduinoOTA.begin();
    #endif

    Log.begin();
    startWdt();

    setupWifiManagerConfigMenu(wm);

    digitalWrite(LED_BL, 1);
    digitalWrite(LED_RT, 0);
    digitalWrite(LED_GN, 0);

    // Set a timeout so the ESP doesn't hang waiting to be configured, for instance after a power failure

    wm.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);

    #ifdef AP_BUTTON_PRESSED
        if (AP_BUTTON_PRESSED) {
            Log.println(F("AP Button pressed during power up"));
            Config.force_ap = true;
        }
    #endif
    #if ENABLE_DOUBLE_RESET == 1
        if (drd->detectDoubleReset()) {
            Log.println(F("Double reset detected"));
            Config.force_ap = true;
        }
    #endif
    if (Config.force_ap) {
        prefs.putBool(ConfigFiles.force_ap, false);
        wm.startConfigPortal("GrowattConfig", APPassword);
        Log.println(F("GrowattConfig finished"));
        digitalWrite(LED_BL, 0);
        delay(3000);
        ESP.restart();
    }

    // Set static ip
    if (!Config.static_ip.isEmpty() && !Config.static_netmask.isEmpty()) {
        IPAddress ip, netmask, gateway, dns;
        ip.fromString(Config.static_ip);
        netmask.fromString(Config.static_netmask);
        gateway.fromString(Config.static_gateway);
        dns.fromString(Config.static_dns);
        Log.print(F("static ip: "));
        Log.println(Config.static_ip);
        Log.print(F("static netmask: "));
        Log.println(Config.static_netmask);
        Log.print(F("static gateway: "));
        Log.println(Config.static_gateway);
        Log.print(F("static dns: "));
        Log.println(Config.static_dns);
        if (!Config.static_dns.isEmpty()) {
            wm.setSTAStaticIPConfig(ip, gateway, netmask, dns);
        } else {
            wm.setSTAStaticIPConfig(ip, gateway, netmask);
        }
    }

    // Automatically connect using saved credentials,
    // if connection fails, it starts an access point with the specified name ("GrowattConfig")
    int connect_timeout_seconds = 15;
    wm.setConnectTimeout(connect_timeout_seconds);
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
        shineMqtt.mqttSetup(Config.mqtt);
    #endif

    httpServer.on("/status", sendJsonSite);
    httpServer.on("/uiStatus", sendUiJsonSite);
    httpServer.on("/metrics", sendMetrics);
    httpServer.on("/startAp", startConfigAccessPoint);
    httpServer.on("/reboot", rebootESP);
    #if ENABLE_MODBUS_COMMUNICATION == 1
    httpServer.on("/postCommunicationModbus", sendPostSite);
    httpServer.on("/postCommunicationModbus_p", HTTP_POST, handlePostData);
    #endif 
    httpServer.on("/", sendMainPage);
    #ifdef ENABLE_WEB_DEBUG
        httpServer.on("/debug", sendDebug);
    #endif
    #if ENABLE_HTTP_COMMAND_ENDPOINT == 1
        const char* Headers[] = {"Content-Type"};
        httpServer.collectHeaders(Headers, sizeof(Headers)/ sizeof(Headers[0]));
    #endif
    httpServer.onNotFound(handleNotFound);

    Inverter.InitProtocol();
    InverterReconnect();
    httpServer.begin();

    #if defined(DEFAULT_NTP_SERVER) && defined(DEFAULT_TZ_INFO)
        #ifdef ESP32
            configTime(0, 0, DEFAULT_NTP_SERVER);
            setenv("TZ", DEFAULT_TZ_INFO, 1);
        #else
            configTime(DEFAULT_TZ_INFO, DEFAULT_NTP_SERVER);
        #endif
    #endif
}


void setupWifiManagerConfigMenu(WiFiManager& wm) {
    customWMParams.hostname = new WiFiManagerParameter("hostname", "hostname (no spaces or special chars)", Config.hostname.c_str(), 30);
    customWMParams.static_ip = new WiFiManagerParameter("staticip", "ip", Config.static_ip.c_str(), 15);
    customWMParams.static_netmask = new WiFiManagerParameter("staticnetmask", "netmask", Config.static_netmask.c_str(), 15);
    customWMParams.static_gateway = new WiFiManagerParameter("staticgateway", "gateway", Config.static_gateway.c_str(), 15);
    customWMParams.static_dns = new WiFiManagerParameter("staticdns", "dns", Config.static_dns.c_str(), 15);
#if MQTT_SUPPORTED == 1
    customWMParams.mqtt_server = new WiFiManagerParameter("mqttserver", "server", Config.mqtt.server.c_str(), 40);
    customWMParams.mqtt_port = new WiFiManagerParameter("mqttport", "port", Config.mqtt.port.c_str(), 6);
    customWMParams.mqtt_topic = new WiFiManagerParameter("mqtttopic", "topic", Config.mqtt.topic.c_str(), 64);
    customWMParams.mqtt_user = new WiFiManagerParameter("mqttusername", "username", Config.mqtt.user.c_str(), 40);
    customWMParams.mqtt_pwd = new WiFiManagerParameter("mqttpassword", "password", Config.mqtt.pwd.c_str(), 64);
#endif
    wm.addParameter(customWMParams.hostname);
#if MQTT_SUPPORTED == 1
    wm.addParameter(new WiFiManagerParameter("<p><b>MQTT Settings</b> (leave server blank to disable)</p>"));
    wm.addParameter(customWMParams.mqtt_server);
    wm.addParameter(customWMParams.mqtt_port);
    wm.addParameter(customWMParams.mqtt_topic);
    wm.addParameter(customWMParams.mqtt_user);
    wm.addParameter(customWMParams.mqtt_pwd);
#endif
    wm.addParameter(new WiFiManagerParameter("<p><b>Static IP</b> (leave blank for DHCP)</p>"));
    wm.addParameter(customWMParams.static_ip);
    wm.addParameter(customWMParams.static_netmask);
    wm.addParameter(customWMParams.static_gateway);
    wm.addParameter(customWMParams.static_dns);

    wm.setSaveParamsCallback(saveParamCallback);

    setupMenu(wm, true);
}

/**
 * @brief create custom wifimanager menu entries
 * 
 * @param enableCustomParams enable custom params aka. mqtt settings
 */
void setupMenu(WiFiManager& wm, bool enableCustomParams){
    Log.println(F("Setting up WiFiManager menu"));
    std::vector<const char*> menu = { "wifi","wifinoscan","update"};
    if(enableCustomParams){
        menu.push_back("param");
    }
    menu.push_back("sep");
    menu.push_back("erase");
    menu.push_back("restart");
    
    wm.setMenu(menu); // custom menu, pass vector
}

void sendJson(JsonDocument& doc)
{
    httpServer.setContentLength(measureJson(doc));
    httpServer.send(200, "application/json", "");
    WiFiClient client = httpServer.client();
    WriteBufferingStream bufferedWifiClient{client, BUFFER_SIZE};
    serializeJson(doc, bufferedWifiClient);
}

void sendJsonSite(void)
{
    if (!readoutSucceeded) {
        httpServer.send(503, F("text/plain"), F("Service Unavailable"));
        return;
    }

    DynamicJsonDocument doc(JSON_DOCUMENT_SIZE);
    Inverter.CreateJson(doc, WiFi.macAddress(), Config.hostname);

    sendJson(doc);
}

void sendUiJsonSite(void)
{
    DynamicJsonDocument doc(JSON_DOCUMENT_SIZE);
    Inverter.CreateUIJson(doc, Config.hostname);

    sendJson(doc);
}

void sendMetrics(void)
{
    if (!readoutSucceeded) {
        httpServer.send(503, F("text/plain"), F("Service Unavailable"));
        return;
    }
    static unsigned maxMetricsSize = 0;
    String metrics;
    if (maxMetricsSize) {
        metrics.reserve(maxMetricsSize);
    }

    Inverter.CreateMetrics(metrics, WiFi.macAddress(), Config.hostname);

    httpServer.setContentLength(metrics.length());
    httpServer.send(200, "text/plain", "");
    WiFiClient client = httpServer.client();
    for (uint16_t i = 0; i < metrics.length(); i += TCP_MSS) {
        int len = min(TCP_MSS, (int)metrics.length() - i);
        client.write(metrics.c_str() + i, len);
    }
    maxMetricsSize = max(maxMetricsSize, metrics.length());
}

#if MQTT_SUPPORTED == 1
boolean sendMqttJson(void)
{
    DynamicJsonDocument doc(JSON_DOCUMENT_SIZE);

    Inverter.CreateJson(doc, WiFi.macAddress(), "");
    return shineMqtt.mqttPublish(doc);
}
#endif

void startConfigAccessPoint(void)
{
    char msg[384];

    snprintf_P(msg, sizeof(msg), PSTR("<html><body>Configuration access point started ...<br /><br />Connect to Wifi: \"GrowattConfig\" with your password (default: \"growsolar\") and visit <a href='http://192.168.4.1'>192.168.4.1</a><br />The Stick will automatically go back to normal operation after a %d seconds</body></html>"), CONFIG_PORTAL_MAX_TIME_SECONDS);
    httpServer.send(200, "text/html", msg);
    delay(2000);
    StartedConfigAfterBoot = true;
}

void rebootESP(void) {
    httpServer.send(200, F("text/html"), F("<html><body>Rebooting...</body></html>"));
    delay(2000);
    ESP.restart();
}

#ifdef ENABLE_WEB_DEBUG
void sendDebug(void) {
    httpServer.sendHeader("Location", "http://" + WiFi.localIP().toString() + ":8080/", true);
    httpServer.send(302, "text/plain", "");
}
#endif

void sendMainPage(void)
{
    httpServer.send(200, "text/html", MAIN_page);
}

void sendPostSite(void)
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
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b input register %ld with value %d"), httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b input register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadInputReg(httpServer.arg(F("reg")).toInt(), &u32Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b input register %ld with value %d"), httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b input register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
            }
            else
            {
                if (httpServer.arg(F("type")) == "16b")
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg(F("reg")).toInt(), &u16Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 16b holding register %ld with value %d"), httpServer.arg("reg").toInt(), u16Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg),PSTR("Read 16b holding register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
                    }
                }
                else
                {
                    if (Inverter.ReadHoldingReg(httpServer.arg(F("reg")).toInt(), &u32Tmp))
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b holding register %ld with value %d"), httpServer.arg("reg").toInt(), u32Tmp);
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Read 32b holding register %ld impossible - not connected?"), httpServer.arg("reg").toInt());
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
                        snprintf_P(msg, sizeof(msg), PSTR("Wrote holding register %ld to a value of %ld!"), httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
                    }
                    else
                    {
                        snprintf_P(msg, sizeof(msg), PSTR("Writing holding register %ld to a value of %ld failed"), httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
                    }
                }
                else
                {
                    snprintf_P(msg, sizeof(msg), PSTR("Writing to double (32b) registers not supported"));
                }
            }
            else
            {
                snprintf_P(msg, sizeof(msg), PSTR("It is not possible to write into input registers"));
            }
        }
        httpServer.send(200, F("text/plain"), msg);
        return;
    }
}

bool sendSingleValue(void)
{
    if (!readoutSucceeded) {
        httpServer.send(503, F("text/plain"), F("Service Unavailable"));
        return true;
    }
    const String& key = httpServer.uri().substring(7);
    double value;
    if (Inverter.GetSingleValueByName(key, value)) {
        httpServer.send(200, "text/plain", String(value));
        return true;
    }
    return false;
}

#if ENABLE_HTTP_COMMAND_ENDPOINT == 1
void handleInverterCommand()
{
    StaticJsonDocument<512> req, res;
    const String& cmd = httpServer.uri().substring(9);
    const String& postData = httpServer.arg(F("plain")).length() > 0 ? httpServer.arg(F("plain")) : F("{}");

    // expect Content-Type application/json to avoid CORS attacks
    bool hasJsonContentType = false;
    for (int i = 0; i < httpServer.headers(); i++) {
        if (httpServer.headerName(i) == F("Content-Type") &&
            httpServer.header(i) == F("application/json")) {
            hasJsonContentType = true;
        }
    }
    if (!hasJsonContentType) {
        httpServer.send(415, F("text/plain"), String(F("Unsupported media type")));
        return;
    }

    Log.print(F("handleInverterCommand: "));
    Log.println(cmd);

    Inverter.HandleCommand(cmd, (byte*) postData.c_str(), postData.length(), req, res);

    httpServer.setContentLength(measureJson(res));
    httpServer.send(200, F("application/json"), "");
    WiFiClient client = httpServer.client();
    WriteBufferingStream bufferedWifiClient{client, BUFFER_SIZE};
    serializeJson(res, bufferedWifiClient);
}
#endif

void handleNotFound() {
    if (httpServer.uri().startsWith(F("/value/")) &&
        httpServer.uri().length() > 7) {
        if (sendSingleValue()) {
            return;
        }
    }
    #if ENABLE_HTTP_COMMAND_ENDPOINT == 1
        if (httpServer.uri().startsWith(F("/command/")) &&
            // Combined with requiring Content-Type application/json this avoids CORS attacks
            httpServer.method() == HTTP_POST &&
            httpServer.uri().length() > 9) {
            handleInverterCommand();
            return;
        }
    #endif
    httpServer.send(404, F("text/plain"), String("Not found: " + httpServer.uri()));
}

#if defined(DEFAULT_NTP_SERVER) && defined(DEFAULT_TZ_INFO)
void handleNTPSync() {
    static unsigned long lastNTPSync = 0;
    unsigned long now = millis();
    // wait 1h between inverter time syncs and wait 60s after
    // ESP startup for Wifi to connect and first NTP sync to happen.
    if (now - lastNTPSync > 3600000 && now > 60000) {
        int reachable = sntp_getreachability(0);
        Log.print(F("NTP server: "));
        Log.print(DEFAULT_NTP_SERVER);
        Log.print(F(" reachable "));
        Log.println(reachable & 1);
        if (reachable & 1) { // last SNTP request was successful
            StaticJsonDocument<128> req, res;
            char buff[32];
            struct tm tm;
            time_t t = time(NULL);
            localtime_r(&t, &tm);
            strftime(buff, sizeof(buff), "{\"value\":\"%Y-%m-%d %T\"}", &tm);
            Log.print(F("Trying to set inverter datetime: "));
            Log.println(buff);
            Inverter.HandleCommand("datetime/set", (byte*) &buff, strlen(buff), req, res);
            Log.println(res["message"].as<String>());
        }
        lastNTPSync = now;
    }
}
#endif

// -------------------------------------------------------
// Main loop
// -------------------------------------------------------
unsigned long ButtonTimer = 0;
unsigned long LEDTimer = 0;
unsigned long RefreshTimer = 0;
unsigned long WifiRetryTimer = 0;

void loop()
{
    #if ENABLE_DOUBLE_RESET
        drd->loop();
    #endif

    Log.loop();
    unsigned long now = millis();

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
        prefs.putBool(ConfigFiles.force_ap, true);
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
            uint8_t u8RetryCounter = NUM_OF_RETRIES;
            readoutSucceeded = false;
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
                    if (shineMqtt.mqttEnabled()) {
                        mqttSuccess = sendMqttJson();
                    }
                    #endif
                    handleWdtReset(mqttSuccess);

                    // leave while-loop
                    readoutSucceeded = true;
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
                    }
                }
            }
        }

        updateRedLed();

        #if PINGER_SUPPORTED == 1
            //frequently check if gateway is reachable
            if (pinger.Ping(GATEWAY_IP) == false) {
                digitalWrite(LED_RT, 1);
                delay(3000);
                ESP.restart();
            }
        #endif

        #if defined(DEFAULT_NTP_SERVER) && defined(DEFAULT_TZ_INFO)
            // set inverter datetime
            handleNTPSync();
        #endif

        RefreshTimer = now;
    }

    #if OTA_SUPPORTED == 1
        // check for OTA updates
        ArduinoOTA.handle();
    #else
        #ifndef ESP32
            // Handle MDNS requests on ESP8266
            MDNS.update();
        #endif
    #endif
}
