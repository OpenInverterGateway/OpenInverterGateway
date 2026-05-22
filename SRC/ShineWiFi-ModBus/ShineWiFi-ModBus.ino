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

#ifdef ESP8266
#include <Updater.h>
#elif defined(ESP32)
#include <Update.h>
#endif

#ifdef ESP32
#include <esp_task_wdt.h>
#endif

#if PINGER_SUPPORTED == 1
#ifdef ESP8266
#include <Pinger.h>
#include <PingerResponse.h>
#else
#include <ESPping.h>  // ESP32 compatible ping library
#endif
#endif

#if ENABLE_DOUBLE_RESET == 1
#define ESP_DRD_USE_LITTLEFS true
#define ESP_DRD_USE_EEPROM false
#define DRD_TIMEOUT 10
#define DRD_ADDRESS 0
#include <ESP_DoubleResetDetector.h>
DoubleResetDetector* drd;
#endif

#if MQTT_SUPPORTED == 1
#include "ShineMqtt.h"
#endif

#if GROWATT_CLOUD_SUPPORTED == 1
#include "GrowattCloud.h"
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

#if GROWATT_CLOUD_SUPPORTED == 1
GrowattCloud growattCloud;
WiFiClient growattCloudClient;
#endif

#ifdef AP_BUTTON_PRESSED
byte btnPressed = 0;
#endif

#define NUM_OF_RETRIES 5
boolean readoutSucceeded = false;

uint16_t u16PacketCnt = 0;
#if PINGER_SUPPORTED == 1
#ifdef ESP8266
Pinger pinger;
#endif
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
#if GROWATT_CLOUD_SUPPORTED == 1
  WiFiManagerParameter* cloud_serial = NULL;
  WiFiManagerParameter* cloud_server = NULL;
#endif
  WiFiManagerParameter* syslog_ip = NULL;
#if ENABLE_WEB_AUTH == 1
  WiFiManagerParameter* auth_user = NULL;
  WiFiManagerParameter* auth_pass = NULL;
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
#if GROWATT_CLOUD_SUPPORTED == 1
  const char* cloud_serial = "/cloudsn";
  const char* cloud_server = "/cloudsrv";
#endif
  const char* syslog_ip = "/syslogip";
#if ENABLE_WEB_AUTH == 1
  const char* auth_user = "/authuser";
  const char* auth_pass = "/authpass";
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
#if GROWATT_CLOUD_SUPPORTED == 1
  String cloud_serial;
  String cloud_server;
#endif
  String syslog_ip;
#if ENABLE_WEB_AUTH == 1
  String auth_user;
  String auth_pass;
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
void WiFi_Reconnect() {
  if (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_GN, 0);

    while (WiFi.status() != WL_CONNECTED) {
      delay(200);
      Log.print(F("x"));
      digitalWrite(LED_RT,
                   !digitalRead(LED_RT));  // toggle red led on WiFi (re)connect
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
// So the detection of the inverter will fail. If no inverter is detected, we
// have to retry later (s. loop() ) The detection without running inverter will
// take several seconds, because the ModBus-Lib has a timeout of 2s for each
// read access (and we do several of them). The WiFi can crash during this
// function. Perhaps we can fix this by using the callback function of the
// ModBus-Lib
void InverterReconnect(void) {
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

void loadConfig() {
  Config.hostname = prefs.getString(ConfigFiles.hostname, DEFAULT_HOSTNAME);
  Config.static_ip = prefs.getString(ConfigFiles.static_ip, "");
  Config.static_netmask = prefs.getString(ConfigFiles.static_netmask, "");
  Config.static_gateway = prefs.getString(ConfigFiles.static_gateway, "");
  Config.static_dns = prefs.getString(ConfigFiles.static_dns, "");
#if MQTT_SUPPORTED == 1
  Config.mqtt.server = prefs.getString(ConfigFiles.mqtt_server, "");
  Config.mqtt.port = prefs.getString(ConfigFiles.mqtt_port, "1883");
  Config.mqtt.topic = prefs.getString(ConfigFiles.mqtt_topic, "energy/solar");
  Config.mqtt.user = prefs.getString(ConfigFiles.mqtt_user, "");
  Config.mqtt.pwd = prefs.getString(ConfigFiles.mqtt_pwd, "");
#endif
#if GROWATT_CLOUD_SUPPORTED == 1
  Config.cloud_serial = prefs.getString(ConfigFiles.cloud_serial, "");
  Config.cloud_server = prefs.getString(ConfigFiles.cloud_server, "server.growatt.com");
#endif
  Config.syslog_ip = prefs.getString(ConfigFiles.syslog_ip, "");
#if ENABLE_WEB_AUTH == 1
  Config.auth_user = prefs.getString(ConfigFiles.auth_user, "");
  Config.auth_pass = prefs.getString(ConfigFiles.auth_pass, "");
#endif
  Config.force_ap = prefs.getBool(ConfigFiles.force_ap, false);
}

void saveConfig() {
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
#if GROWATT_CLOUD_SUPPORTED == 1
  prefs.putString(ConfigFiles.cloud_serial, Config.cloud_serial);
  prefs.putString(ConfigFiles.cloud_server, Config.cloud_server);
#endif
  prefs.putString(ConfigFiles.syslog_ip, Config.syslog_ip);
#if ENABLE_WEB_AUTH == 1
  prefs.putString(ConfigFiles.auth_user, Config.auth_user);
  prefs.putString(ConfigFiles.auth_pass, Config.auth_pass);
#endif
}

void saveParamCallback() {
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
#if GROWATT_CLOUD_SUPPORTED == 1
  Config.cloud_serial = customWMParams.cloud_serial->getValue();
  Config.cloud_server = customWMParams.cloud_server->getValue();
  if (Config.cloud_server.isEmpty()) {
    Config.cloud_server = "server.growatt.com";
  }
#endif
  Config.syslog_ip = customWMParams.syslog_ip->getValue();
#if ENABLE_WEB_AUTH == 1
  Config.auth_user = customWMParams.auth_user->getValue();
  {
    String val = customWMParams.auth_pass->getValue();
    if (!val.isEmpty()) Config.auth_pass = val;
  }
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

#include <SyslogStream.h>
SyslogStream syslogStream = SyslogStream();

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
  if (!Config.syslog_ip.isEmpty()) {
    syslogStream.setDestination(Config.syslog_ip.c_str());
    // syslogStream.setRaw(true);
    const std::shared_ptr<LOGBase> syslogStreamPtr =
        std::make_shared<SyslogStream>(syslogStream);
    Log.addPrintStream(syslogStreamPtr);
    Log.print(F("syslog server ip: "));
    Log.println(Config.syslog_ip);
  }
}

void setupGPIO() {
  pinMode(LED_GN, OUTPUT);
  pinMode(LED_RT, OUTPUT);
  pinMode(LED_BL, OUTPUT);
}

void setupWifiHost() {
#ifdef ESP32
  // ESP32 needs this here (before WiFi.mode) for core 2.0.0
  WiFi.hostname(Config.hostname);
#endif
  WiFi.mode(WIFI_STA);  // explicitly set mode, esp defaults to STA+AP
#ifdef ESP8266
  // ESP8266 needs this here (after WiFi.mode)
  WiFi.hostname(Config.hostname);
#endif
#if OTA_SUPPORTED == 0
  MDNS.begin(Config.hostname);
#endif
  Log.print(F("setupWifiHost: hostname "));
  Log.println(Config.hostname);
}

void startWdt() {
#ifdef ESP32
  Log.println("Configuring WDT...");
  esp_task_wdt_init(WDT_TIMEOUT, true);
  esp_task_wdt_add(NULL);
#endif
}

void handleWdtReset(boolean mqttSuccess) {
#if MQTT_SUPPORTED == 1
  if (mqttSuccess) {
    resetWdt();
  } else {
    if (!shineMqtt.mqttEnabled()) {
      resetWdt();
    }
  }
#else
  resetWdt();
#endif
}

void resetWdt() {
#ifdef ESP32
  Log.println(F("WDT timer restart ..."));
  esp_task_wdt_reset();
#endif
}

void setup() {
  WiFiManager wm;

  Log.println("Setup()");

  setupGPIO();

#if ENABLE_DOUBLE_RESET == 1
  drd = new DoubleResetDetector(DRD_TIMEOUT, DRD_ADDRESS);
#endif

  prefs.begin("ShineWifi");

  loadConfig();
  configureLogging();
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

  // Set a timeout so the ESP doesn't hang waiting to be configured, for
  // instance after a power failure

  wm.setConfigPortalTimeout(CONFIG_PORTAL_MAX_TIME_SECONDS);

  Log.print("force_ap: ");
  Log.println(Config.force_ap);

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
  // if connection fails, it starts an access point with the specified name
  // ("GrowattConfig")
  int connect_timeout_seconds = 15;
  wm.setConnectTimeout(connect_timeout_seconds);
  bool res = wm.autoConnect("GrowattConfig",
                            APPassword);  // password protected wificonfig ap

  if (!res) {
    Log.println(F("Failed to connect WIFI"));
    ESP.restart();
  } else {
    digitalWrite(LED_BL, 0);
    // if you get here you have connected to the WiFi
    Log.println(F("WIFI connected...yeey :)"));
  }

  while (WiFi.status() != WL_CONNECTED) {
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
  httpServer.on("/config", HTTP_GET, sendConfigJson);
  httpServer.on("/saveConfig", HTTP_POST, handleSaveConfig);
  httpServer.on("/cloudStatus", sendCloudStatus);
  httpServer.on("/systemStatus", sendSystemStatus);
  httpServer.on("/update", HTTP_GET, sendUpdatePage);
  httpServer.on("/doUpdate", HTTP_POST, handleUpdateDone, handleUpdateUpload);
  httpServer.onNotFound(handleNotFound);

  Inverter.InitProtocol();
  InverterReconnect();
  // Collect Origin and Referer so checkSameOrigin() can inspect them.
  const char* csrfHeaders[] = {"Origin", "Referer"};
  httpServer.collectHeaders(csrfHeaders, sizeof(csrfHeaders) / sizeof(csrfHeaders[0]));
  httpServer.begin();

#if GROWATT_CLOUD_SUPPORTED == 1
  // Try to auto-read serial from stock firmware's flash area (0x3FD0B4)
  // This survives OIG flashing since we only write the first ~500KB
  if (Config.cloud_serial.isEmpty()) {
    uint8_t flashBuf[12] = {0};
    char flashSerial[11] = {0};
    bool found = false;
#ifdef ESP8266
    if (ESP.flashRead(0x3FD0B4, (uint32_t*)flashBuf, 12)) {
      memcpy(flashSerial, flashBuf, 10);
      flashSerial[10] = '\0';
      found = true;
      for (int i = 0; i < 10 && found; i++) {
        if (flashSerial[i] < 0x20 || flashSerial[i] > 0x7E) {
          if (i == 0) found = false;
          else flashSerial[i] = '\0';
        }
      }
    }
#endif
    if (found && strlen(flashSerial) >= 6) {
      Config.cloud_serial = String(flashSerial);
      saveConfig();
      Log.print(F("[GrowattCloud] Auto-detected serial from flash: "));
      Log.println(Config.cloud_serial);
    }
  }

  if (!Config.cloud_serial.isEmpty()) {
    GrowattCloudConfig cloudCfg = {};
    strncpy(cloudCfg.serverHost, Config.cloud_server.c_str(), sizeof(cloudCfg.serverHost) - 1);
    cloudCfg.serverHost[sizeof(cloudCfg.serverHost) - 1] = '\0';
    cloudCfg.serverPort = GROWATT_PORT_DEFAULT;
    cloudCfg.protocolId = GROWATT_PROTO_ENC_V2;
    strncpy(cloudCfg.dataloggerSerial, Config.cloud_serial.c_str(), sizeof(cloudCfg.dataloggerSerial) - 1);
    cloudCfg.dataloggerSerial[sizeof(cloudCfg.dataloggerSerial) - 1] = '\0';
    // inverterSerial already zeroed by the {} initializer above
    strncpy(cloudCfg.fwVersion, "1.7.7.7", sizeof(cloudCfg.fwVersion) - 1);
    cloudCfg.fwVersion[sizeof(cloudCfg.fwVersion) - 1] = '\0';
    cloudCfg.logIntervalMin = 5;
    cloudCfg.enabled = true;
    growattCloud.begin(cloudCfg);
    Log.println(F("[GrowattCloud] Enabled, serial: "));
    Log.println(Config.cloud_serial);
  } else {
    Log.println(F("[GrowattCloud] Disabled (no serial configured)"));
  }
#endif

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
  customWMParams.hostname = new WiFiManagerParameter(
      "hostname", "hostname (no spaces or special chars)",
      Config.hostname.c_str(), 30);
  customWMParams.static_ip =
      new WiFiManagerParameter("staticip", "ip", Config.static_ip.c_str(), 15);
  customWMParams.static_netmask = new WiFiManagerParameter(
      "staticnetmask", "netmask", Config.static_netmask.c_str(), 15);
  customWMParams.static_gateway = new WiFiManagerParameter(
      "staticgateway", "gateway", Config.static_gateway.c_str(), 15);
  customWMParams.static_dns = new WiFiManagerParameter(
      "staticdns", "dns", Config.static_dns.c_str(), 15);
#if MQTT_SUPPORTED == 1
  customWMParams.mqtt_server = new WiFiManagerParameter(
      "mqttserver", "server", Config.mqtt.server.c_str(), 40);
  customWMParams.mqtt_port =
      new WiFiManagerParameter("mqttport", "port", Config.mqtt.port.c_str(), 6);
  customWMParams.mqtt_topic = new WiFiManagerParameter(
      "mqtttopic", "topic", Config.mqtt.topic.c_str(), 64);
  customWMParams.mqtt_user = new WiFiManagerParameter(
      "mqttusername", "username", Config.mqtt.user.c_str(), 40);
  customWMParams.mqtt_pwd = new WiFiManagerParameter(
      "mqttpassword", "password", Config.mqtt.pwd.c_str(), 64);
#endif
  customWMParams.syslog_ip = new WiFiManagerParameter(
      "syslogip", "syslog server IP (leave blank for none)",
      Config.syslog_ip.c_str(), 15);
  wm.addParameter(customWMParams.hostname);
#if MQTT_SUPPORTED == 1
  wm.addParameter(new WiFiManagerParameter(
      "<p><b>MQTT Settings</b> (leave server blank to disable)</p>"));
  wm.addParameter(customWMParams.mqtt_server);
  wm.addParameter(customWMParams.mqtt_port);
  wm.addParameter(customWMParams.mqtt_topic);
  wm.addParameter(customWMParams.mqtt_user);
  wm.addParameter(customWMParams.mqtt_pwd);
#endif
#if GROWATT_CLOUD_SUPPORTED == 1
  customWMParams.cloud_serial = new WiFiManagerParameter(
      "cloudsn", "Datalogger Serial (from stick label/AP SSID)",
      Config.cloud_serial.c_str(), 30);
  customWMParams.cloud_server = new WiFiManagerParameter(
      "cloudsrv", "Cloud Server (default: server.growatt.com)",
      Config.cloud_server.c_str(), 60);
  wm.addParameter(new WiFiManagerParameter(
      "<p><b>Growatt Cloud</b> (enter serial to enable, leave blank to disable)</p>"));
  wm.addParameter(customWMParams.cloud_serial);
  wm.addParameter(customWMParams.cloud_server);
#endif
  wm.addParameter(new WiFiManagerParameter(
      "<p><b>Static IP</b> (leave blank for DHCP)</p>"));
  wm.addParameter(customWMParams.static_ip);
  wm.addParameter(customWMParams.static_netmask);
  wm.addParameter(customWMParams.static_gateway);
  wm.addParameter(customWMParams.static_dns);
  wm.addParameter(new WiFiManagerParameter("<p><b>Advanced Settings</b></p>"));
  wm.addParameter(customWMParams.syslog_ip);
#if ENABLE_WEB_AUTH == 1
  customWMParams.auth_user = new WiFiManagerParameter(
      "authuser", "Web UI Username (blank=no auth)",
      Config.auth_user.c_str(), 30);
  customWMParams.auth_pass = new WiFiManagerParameter(
      "authpass", "Web UI Password",
      "", 30);  // Don't show current password
  wm.addParameter(new WiFiManagerParameter(
      "<p><b>Web Authentication</b> (leave blank for open access)</p>"));
  wm.addParameter(customWMParams.auth_user);
  wm.addParameter(customWMParams.auth_pass);
#endif

  wm.setSaveParamsCallback(saveParamCallback);

  setupMenu(wm, true);
}

/**
 * @brief create custom wifimanager menu entries
 *
 * @param enableCustomParams enable custom params aka. mqtt settings
 */
void setupMenu(WiFiManager& wm, bool enableCustomParams) {
  Log.println(F("Setting up WiFiManager menu"));
  std::vector<const char*> menu = {"wifi", "wifinoscan", "update"};
  if (enableCustomParams) {
    menu.push_back("param");
  }
  menu.push_back("sep");
  menu.push_back("erase");
  menu.push_back("restart");

  wm.setMenu(menu);  // custom menu, pass vector
}

bool checkAuth() {
#if ENABLE_WEB_AUTH == 1
  if (Config.auth_user.isEmpty() || Config.auth_pass.isEmpty()) {
    return true;
  }
  if (!httpServer.authenticate(Config.auth_user.c_str(), Config.auth_pass.c_str())) {
    httpServer.requestAuthentication(BASIC_AUTH, "Growatt Inverter");
    return false;
  }
#endif
  return true;
}

// CSRF mitigation for state-changing endpoints. Browsers send Basic Auth
// credentials automatically, so a cross-site POST could otherwise reconfigure
// the device. Require that the Origin (or Referer) header points at this
// device. Same-origin form posts from the device's own pages always include
// one of these; cross-site posts will not (or will point elsewhere).
bool checkSameOrigin() {
  String host = httpServer.hostHeader();
  String origin = httpServer.header(F("Origin"));
  String referer = httpServer.header(F("Referer"));
  auto matches = [&host](const String& url) -> bool {
    if (url.isEmpty() || host.isEmpty()) return false;
    int schemeEnd = url.indexOf(F("://"));
    if (schemeEnd < 0) return false;
    int hostStart = schemeEnd + 3;
    int hostEnd = url.indexOf('/', hostStart);
    String urlHost = (hostEnd < 0) ? url.substring(hostStart)
                                   : url.substring(hostStart, hostEnd);
    return urlHost.equalsIgnoreCase(host);
  };
  if (!origin.isEmpty()) return matches(origin);
  if (!referer.isEmpty()) return matches(referer);
  // Neither header present: reject state-changing request to be safe.
  return false;
}

void sendJson(JsonDocument& doc) {
  httpServer.setContentLength(measureJson(doc));
  httpServer.send(200, "application/json", "");
  WiFiClient client = httpServer.client();
  WriteBufferingStream bufferedWifiClient{client, BUFFER_SIZE};
  serializeJson(doc, bufferedWifiClient);
}

void sendJsonSite(void) {
  if (!checkAuth()) return;
  if (!readoutSucceeded) {
    httpServer.send(503, F("text/plain"), F("Service Unavailable"));
    return;
  }

  DynamicJsonDocument doc(JSON_DOCUMENT_SIZE);
  Inverter.CreateJson(doc, WiFi.macAddress(), Config.hostname);

  sendJson(doc);
}

void sendUiJsonSite(void) {
  if (!checkAuth()) return;
  DynamicJsonDocument doc(JSON_DOCUMENT_SIZE);
  Inverter.CreateUIJson(doc, Config.hostname);

  sendJson(doc);
}

void sendMetrics(void) {
  if (!checkAuth()) return;
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
boolean sendMqttJson(void) {
  DynamicJsonDocument doc(JSON_DOCUMENT_SIZE);

  Inverter.CreateJson(doc, WiFi.macAddress(), "");
  return shineMqtt.mqttPublish(doc);
}
#endif

void startConfigAccessPoint(void) {
  if (!checkAuth()) return;
  char msg[384];

  snprintf_P(msg, sizeof(msg),
             PSTR("<html><body>Configuration access point started ...<br /><br "
                  "/>Connect to Wifi: \"GrowattConfig\" with your password "
                  "(default: \"growsolar\") and visit <a "
                  "href='http://192.168.4.1'>192.168.4.1</a><br />The Stick "
                  "will automatically go back to normal operation after a %d "
                  "seconds</body></html>"),
             CONFIG_PORTAL_MAX_TIME_SECONDS);
  httpServer.send(200, "text/html", msg);
  delay(2000);
  StartedConfigAfterBoot = true;
}

void rebootESP(void) {
  if (!checkAuth()) return;
  httpServer.send(200, F("text/html"),
                  F("<html><body>Rebooting...</body></html>"));
  delay(2000);
  ESP.restart();
}

#ifdef ENABLE_WEB_DEBUG
void sendDebug(void) {
  if (!checkAuth()) return;
  httpServer.sendHeader("Location",
                        "http://" + WiFi.localIP().toString() + ":8080/", true);
  httpServer.send(302, "text/plain", "");
}
#endif

void sendMainPage(void) {
  if (!checkAuth()) return;
  httpServer.send(200, "text/html", MAIN_page);
}

void sendPostSite(void) {
  if (!checkAuth()) return;
  httpServer.send(200, "text/html", SendPostSite_page);
}

void handlePostData() {
  if (!checkAuth()) return;
  char msg[256];
  uint16_t u16Tmp;
  uint32_t u32Tmp;

  if (!httpServer.hasArg(F("reg")) || !httpServer.hasArg(F("val"))) {
    // If the POST request doesn't have data
    httpServer.send(400, F("text/plain"),
                    F("400: Invalid Request"));  // The request is invalid, so
                                                 // send HTTP status 400
    return;
  } else {
    if (httpServer.arg(F("operation")) == "R") {
      if (httpServer.arg(F("registerType")) == "I") {
        if (httpServer.arg(F("type")) == "16b") {
          if (Inverter.ReadInputReg(httpServer.arg(F("reg")).toInt(),
                                    &u16Tmp)) {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Read 16b input register %ld with value %d"),
                       httpServer.arg("reg").toInt(), u16Tmp);
          } else {
            snprintf_P(
                msg, sizeof(msg),
                PSTR("Read 16b input register %ld impossible - not connected?"),
                httpServer.arg("reg").toInt());
          }
        } else {
          if (Inverter.ReadInputReg(httpServer.arg(F("reg")).toInt(),
                                    &u32Tmp)) {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Read 32b input register %ld with value %d"),
                       httpServer.arg("reg").toInt(), u32Tmp);
          } else {
            snprintf_P(
                msg, sizeof(msg),
                PSTR("Read 32b input register %ld impossible - not connected?"),
                httpServer.arg("reg").toInt());
          }
        }
      } else {
        if (httpServer.arg(F("type")) == "16b") {
          if (Inverter.ReadHoldingReg(httpServer.arg(F("reg")).toInt(),
                                      &u16Tmp)) {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Read 16b holding register %ld with value %d"),
                       httpServer.arg("reg").toInt(), u16Tmp);
          } else {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Read 16b holding register %ld impossible - not "
                            "connected?"),
                       httpServer.arg("reg").toInt());
          }
        } else {
          if (Inverter.ReadHoldingReg(httpServer.arg(F("reg")).toInt(),
                                      &u32Tmp)) {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Read 32b holding register %ld with value %d"),
                       httpServer.arg("reg").toInt(), u32Tmp);
          } else {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Read 32b holding register %ld impossible - not "
                            "connected?"),
                       httpServer.arg("reg").toInt());
          }
        }
      }
    } else {
      if (httpServer.arg(F("registerType")) == "H") {
        if (httpServer.arg(F("type")) == "16b") {
          if (Inverter.WriteHoldingReg(httpServer.arg(F("reg")).toInt(),
                                       httpServer.arg(F("val")).toInt())) {
            snprintf_P(msg, sizeof(msg),
                       PSTR("Wrote holding register %ld to a value of %ld!"),
                       httpServer.arg("reg").toInt(),
                       httpServer.arg("val").toInt());
          } else {
            snprintf_P(
                msg, sizeof(msg),
                PSTR("Writing holding register %ld to a value of %ld failed"),
                httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
          }
        } else {
          snprintf_P(msg, sizeof(msg),
                     PSTR("Writing to double (32b) registers not supported"));
        }
      } else {
        snprintf_P(msg, sizeof(msg),
                   PSTR("It is not possible to write into input registers"));
      }
    }
    httpServer.send(200, F("text/plain"), msg);
    return;
  }
}

// -------------------------------------------------------
// OTA Firmware Update via Web UI
// -------------------------------------------------------
void sendUpdatePage(void) {
  if (!checkAuth()) return;
  httpServer.send(200, F("text/html"),
    F("<!DOCTYPE html><html><head><meta charset='utf-8'><meta name='viewport' content='width=device-width,initial-scale=1'>"
      "<title>Firmware Update</title>"
      "<link rel='stylesheet' href='https://cdn.jsdelivr.net/npm/@picocss/pico@2/css/pico.min.css'>"
      "</head><body><main class='container'>"
      "<h2>Firmware Update</h2>"
      "<form method='POST' action='/doUpdate' enctype='multipart/form-data'>"
      "<label>Select firmware .bin file:<input type='file' name='update' accept='.bin' required></label>"
      "<button type='submit'>Upload and Flash</button>"
      "</form>"
      "<p><a href='/'>Back to Dashboard</a></p>"
      "</main></body></html>"));
}

void handleUpdateUpload(void) {
  if (!checkAuth()) return;
  HTTPUpload& upload = httpServer.upload();
  if (upload.status == UPLOAD_FILE_START) {
    Log.printf("OTA Update: %s (%u bytes)\n", upload.filename.c_str(), upload.totalSize);
    uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
    if (!Update.begin(maxSketchSpace)) {
      Log.println(F("OTA: Not enough space"));
    }
  } else if (upload.status == UPLOAD_FILE_WRITE) {
    if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
      Log.println(F("OTA: Write failed"));
    }
  } else if (upload.status == UPLOAD_FILE_END) {
    if (Update.end(true)) {
      Log.printf("OTA: Success, %u bytes\n", upload.totalSize);
    } else {
      Log.println(F("OTA: Failed"));
    }
  }
  yield();
}

void handleUpdateDone(void) {
  if (!checkAuth()) return;
  bool ok = !Update.hasError();
  httpServer.send(200, F("text/html"),
    ok ? F("<!DOCTYPE html><html><body><h2>Update Successful!</h2><p>Rebooting...</p></body></html>")
       : F("<!DOCTYPE html><html><body><h2>Update Failed!</h2><p><a href='/update'>Try again</a></p></body></html>"));
  if (ok) {
    delay(1000);
    ESP.restart();
  }
}

void sendConfigJson(void) {
  if (!checkAuth()) return;
  StaticJsonDocument<512> doc;
  doc["hostname"] = Config.hostname;
  doc["static_ip"] = Config.static_ip;
  doc["static_netmask"] = Config.static_netmask;
  doc["static_gateway"] = Config.static_gateway;
  doc["static_dns"] = Config.static_dns;
#if MQTT_SUPPORTED == 1
  doc["mqtt_server"] = Config.mqtt.server;
  doc["mqtt_port"] = Config.mqtt.port;
  doc["mqtt_topic"] = Config.mqtt.topic;
  doc["mqtt_user"] = Config.mqtt.user;
  doc["mqtt_pwd_set"] = !Config.mqtt.pwd.isEmpty();
#endif
#if GROWATT_CLOUD_SUPPORTED == 1
  doc["cloud_serial"] = Config.cloud_serial;
  doc["cloud_server"] = Config.cloud_server;
#endif
  doc["syslog_ip"] = Config.syslog_ip;
#if ENABLE_WEB_AUTH == 1
  doc["auth_user"] = Config.auth_user;
  doc["auth_pwd_set"] = !Config.auth_pass.isEmpty();
#endif
  sendJson(doc);
}

void handleSaveConfig(void) {
  if (!checkAuth()) return;
  if (!checkSameOrigin()) {
    httpServer.send(403, F("text/plain"), F("Forbidden: cross-origin request rejected"));
    return;
  }
  if (httpServer.hasArg(F("hostname"))) Config.hostname = httpServer.arg(F("hostname"));
  if (httpServer.hasArg(F("static_ip"))) Config.static_ip = httpServer.arg(F("static_ip"));
  if (httpServer.hasArg(F("static_netmask"))) Config.static_netmask = httpServer.arg(F("static_netmask"));
  if (httpServer.hasArg(F("static_gateway"))) Config.static_gateway = httpServer.arg(F("static_gateway"));
  if (httpServer.hasArg(F("static_dns"))) Config.static_dns = httpServer.arg(F("static_dns"));
#if MQTT_SUPPORTED == 1
  if (httpServer.hasArg(F("mqtt_server"))) Config.mqtt.server = httpServer.arg(F("mqtt_server"));
  if (httpServer.hasArg(F("mqtt_port"))) Config.mqtt.port = httpServer.arg(F("mqtt_port"));
  if (httpServer.hasArg(F("mqtt_topic"))) Config.mqtt.topic = httpServer.arg(F("mqtt_topic"));
  if (httpServer.hasArg(F("mqtt_user"))) Config.mqtt.user = httpServer.arg(F("mqtt_user"));
  if (httpServer.hasArg(F("mqtt_pwd"))) {
    String val = httpServer.arg(F("mqtt_pwd"));
    if (!val.isEmpty()) Config.mqtt.pwd = val;
  }
#endif
#if GROWATT_CLOUD_SUPPORTED == 1
  if (httpServer.hasArg(F("cloud_serial"))) Config.cloud_serial = httpServer.arg(F("cloud_serial"));
  if (httpServer.hasArg(F("cloud_server"))) {
    String val = httpServer.arg(F("cloud_server"));
    Config.cloud_server = val.isEmpty() ? "server.growatt.com" : val;
  }
#endif
  if (httpServer.hasArg(F("syslog_ip"))) Config.syslog_ip = httpServer.arg(F("syslog_ip"));
#if ENABLE_WEB_AUTH == 1
  if (httpServer.hasArg(F("auth_user"))) Config.auth_user = httpServer.arg(F("auth_user"));
  if (httpServer.hasArg(F("auth_pass"))) {
    String val = httpServer.arg(F("auth_pass"));
    if (!val.isEmpty()) Config.auth_pass = val;
  }
#endif
  saveConfig();
  bool needRestart = httpServer.hasArg(F("restart"));
  httpServer.send(200, F("application/json"),
    needRestart ? F("{\"ok\":true,\"restart\":true}") : F("{\"ok\":true}"));
  if (needRestart) {
    delay(1000);
    ESP.restart();
  }
}

void sendCloudStatus(void) {
  if (!checkAuth()) return;
  StaticJsonDocument<256> doc;
#if GROWATT_CLOUD_SUPPORTED == 1
  if (!Config.cloud_serial.isEmpty()) {
    doc["enabled"] = true;
    static const char* const stateNames[] = {"Disconnected","Connecting","Connected","Announced","Active","Error"};
    int sIdx = (int)growattCloud.getState();
    doc["state"] = (sIdx >= 0 && sIdx < (int)(sizeof(stateNames)/sizeof(stateNames[0])))
                      ? stateNames[sIdx] : "Unknown";
    doc["stateCode"] = sIdx;
    doc["packetsSent"] = growattCloud.getPacketsSent();
    doc["packetsRecv"] = growattCloud.getPacketsRecv();
    doc["reconnects"] = growattCloud.getReconnects();
    uint32_t lastSend = growattCloud.getLastSendTime();
    doc["lastSendAgo"] = lastSend > 0 ? (int)((millis() - lastSend) / 1000) : -1;
    doc["serial"] = Config.cloud_serial;
    doc["server"] = Config.cloud_server;
  } else {
    doc["enabled"] = false;
  }
#else
  doc["supported"] = false;
#endif
  sendJson(doc);
}

void sendSystemStatus(void) {
  if (!checkAuth()) return;
  StaticJsonDocument<384> doc;
  doc["hostname"] = Config.hostname;
  doc["ip"] = WiFi.localIP().toString();
  doc["gateway"] = WiFi.gatewayIP().toString();
  doc["netmask"] = WiFi.subnetMask().toString();
  doc["dns"] = WiFi.dnsIP().toString();
  doc["ssid"] = WiFi.SSID();
  doc["rssi"] = WiFi.RSSI();
  doc["heap"] = ESP.getFreeHeap();
  doc["uptime"] = millis() / 1000;
  doc["mac"] = WiFi.macAddress();
  doc["stickType"] = Inverter.GetWiFiStickType() == ShineWiFi_S ? "ShineWiFi-S" :
                     Inverter.GetWiFiStickType() == ShineWiFi_X ? "ShineWiFi-X" : "Unknown";
#if GROWATT_CLOUD_SUPPORTED == 1
  doc["cloudSerial"] = Config.cloud_serial;
  doc["cloudState"] = growattCloud.isConnected() ? "Active" : "Disconnected";
#endif
  sendJson(doc);
}

bool sendSingleValue(void) {
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

void handleNotFound() {
  if (!checkAuth()) return;
  if (httpServer.uri().startsWith(F("/value/")) &&
      httpServer.uri().length() > 7) {
    if (sendSingleValue()) {
      return;
    }
  }
  httpServer.send(404, F("text/plain"),
                  String("Not found: " + httpServer.uri()));
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
    if (reachable & 1) {  // last SNTP request was successful
      StaticJsonDocument<128> req, res;
      char buff[32];
      struct tm tm;
      time_t t = time(NULL);
      localtime_r(&t, &tm);
      strftime(buff, sizeof(buff), "{\"value\":\"%Y-%m-%d %T\"}", &tm);
      Log.print(F("Trying to set inverter datetime: "));
      Log.println(buff);
      Inverter.HandleCommand("datetime/set", (byte*)&buff, strlen(buff), req,
                             res);
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

void loop() {
#if ENABLE_DOUBLE_RESET
  drd->loop();
#endif

  Log.loop();
  unsigned long now = millis();

#ifdef AP_BUTTON_PRESSED
  if ((now - ButtonTimer) > BUTTON_TIMER) {
    ButtonTimer = now;

    if (AP_BUTTON_PRESSED) {
      if (btnPressed > 5) {
        Log.println(F("Handle press"));
        StartedConfigAfterBoot = true;
      } else {
        btnPressed++;
      }
      Log.print(F("Btn pressed"));
    } else {
      btnPressed = 0;
    }
  }
#endif

  if (StartedConfigAfterBoot == true) {
    Log.println("StartedConfigAfterBoot");
    prefs.putBool(ConfigFiles.force_ap, true);
    delay(3000);
    ESP.restart();
  }

  WiFi_Reconnect();

#if MQTT_SUPPORTED == 1
  if (shineMqtt.mqttReconnect()) {
    shineMqtt.loop();
  }
#endif

  httpServer.handleClient();

  // Toggle green LED with 1 Hz (alive)
  // ------------------------------------------------------------
  if ((now - LEDTimer) > LED_TIMER) {
    if (WiFi.status() == WL_CONNECTED)
      digitalWrite(LED_GN, !digitalRead(LED_GN));
    else
      digitalWrite(LED_GN, 0);

    LEDTimer = now;
  }

  // InverterReconnect() takes a long time --> wifi will crash
  // Do it only every two minutes
  if ((now - WifiRetryTimer) > WIFI_RETRY_TIMER) {
    if (Inverter.GetWiFiStickType() == Undef_stick) InverterReconnect();
    WifiRetryTimer = now;
  }

  // Read Inverter every REFRESH_TIMER ms [defined in config.h]
  // ------------------------------------------------------------
  if ((now - RefreshTimer) > REFRESH_TIMER) {
    if ((WiFi.status() == WL_CONNECTED) && (Inverter.GetWiFiStickType())) {
      uint8_t u8RetryCounter = NUM_OF_RETRIES;
      readoutSucceeded = false;
      while ((u8RetryCounter) && !(readoutSucceeded)) {
#if SIMULATE_INVERTER == 1
        if (1)  // do it always
#else
        if (Inverter.ReadData())  // get new data from inverter
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

#if GROWATT_CLOUD_SUPPORTED == 1
          // Feed register data to Growatt Cloud sender
          if (!Config.cloud_serial.isEmpty()) {
            // Build arrays of raw register values for cloud protocol.
            // Cap counts to the local buffer sizes so the downstream callee
            // never reads past inputRegs[]/holdingRegs[].
            uint16_t inputRegs[125];
            uint16_t holdingRegs[35];
            uint16_t numInput = Inverter._Protocol.InputRegisterCount;
            uint16_t numHolding = Inverter._Protocol.HoldingRegisterCount;
            if (numInput > 125) numInput = 125;
            if (numHolding > 35) numHolding = 35;
            for (uint16_t i = 0; i < numInput; i++) {
              inputRegs[i] = (uint16_t)Inverter._Protocol.InputRegisters[i].value;
            }
            for (uint16_t i = 0; i < numHolding; i++) {
              holdingRegs[i] = (uint16_t)Inverter._Protocol.HoldingRegisters[i].value;
            }
            growattCloud.loop(inputRegs, numInput, holdingRegs, numHolding);
          }
#endif

          // leave while-loop
          readoutSucceeded = true;
        } else {
          Log.println(F("ReadData() NOT successful"));
          if (u8RetryCounter) {
            u8RetryCounter--;
          } else {
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
    // frequently check if gateway is reachable
    bool pingSuccess = false;
#ifdef ESP8266
    pingSuccess = pinger.Ping(GATEWAY_IP);
#else
    // ESP32 uses ESPping library with different API
    // ping(host, count, interval_ms, size, timeout_ms)
    String gatewayIP = GATEWAY_IP.toString();
    ping(gatewayIP.c_str(), 1, 10, 32, 5000);  // Single ping with 5s timeout
    // Note: ESPping library doesn't return bool, it prints to Serial
    // For now, we'll skip the restart logic for ESP32
    pingSuccess = true;  // Assume success for ESP32
#endif

    if (!pingSuccess) {
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

#if GROWATT_CLOUD_SUPPORTED == 1
  // Keep Growatt Cloud connection alive (pings, server responses)
  if (!Config.cloud_serial.isEmpty()) {
    growattCloud.loop(nullptr, 0);
  }
#endif

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
