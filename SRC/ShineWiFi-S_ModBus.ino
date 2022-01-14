/*
Datei -> Voreinstellungen -> Zusaetzliche Boardverwalter-URLs -> "http://arduino.esp8266.com/stable/package_esp8266com_index.json"
Werkzeuge -> Board -> Boardverwalter -> ESP8266

Used Libs
----------

Download MQTT Client from
https://github.com/knolleary/pubsubclient
Install --> Sketch -> Bibliothek einbinden -> .Zib Bibliotherk hinzufuegen

Download ModbusMaster by Doc Walker

Board: Generic ESP8266 Module
Flash Mode: DIO
Cristal Freq:: 26 MHz
Flash Freq: 40 MHz
Upload Using: Serial
CPU Freq: 80 MHz
Flash Size: 4 MB (FS:none OTA~1019KB)
UploadSpeed: 115200

Thanks to Jethro Kairys
https://github.com/jkairys/growatt-esp8266

File -> "Show verbose output during:" "compilation".
This will show the path to the binary during compilation 
e.g. C:\Users\<username>\AppData\Local\Temp\arduino_build_533155


2019-10-16
*/

// ---------------------------------------------------------------
// User configuration area start
// ---------------------------------------------------------------

// Setting this define to 0 will disable the MQTT functionality
#define MQTT_SUPPORTED 1

// Setting this define to 1 will ping the default gateway periodically 
// if the ping is not successful, the wifi connection will be reestablished
#define PINGER_SUPPORTED 0

// Setting this define to 1 will enable the debug output via the serial port.
// The serial port is the same used for the communication to the inverter.
// Enabling this feature can cause problems with the inverter communication!
// For ShineWiFi-S everything seems to work perfect even though this flag is set
#define ENABLE_DEBUG_OUTPUT 1

// Setting this define to 1 will enable a web page (<ip>/debug) where debug messages can be displayed
#define ENABLE_WEB_DEBUG 0


// Setting this flag to 1 will simulate the inverter
// This could be helpful if it is night and the inverter is not working or
// during development where the stick is not connected to the inverter
#define SIMULATE_INVERTER 0

// Data of the Wifi access point
#define WIFI_SSID         "<WIFI_SSID>"
#define WIFI_PASSWORD     "<WIFI_PASSWORD>"
#define HOSTNAME          "Growatt"

// Username and password for firmware update
#define UPDATE_USER       "<UDATE_USER>"
#define UPDATE_PASSWORD   "<UPDATE_PASSWORD>"

#if MQTT_SUPPORTED == 1 
#define MQTT_SERVER       "<MQTT_SERVER_IP>"
#define MQTT_TOPIC        "<MyTopic>"
#endif

#if PINGER_SUPPORTED == 1
#define GATEWAY_IP IPAddress(192, 168, 178, 1)
#endif

#if ENABLE_WEB_DEBUG == 1
    char acWebDebug[1024] = "";
    uint16_t u16WebMsgNo = 0;
    #define WEB_DEBUG_PRINT(s) {if( (strlen(acWebDebug)+strlen(s)+50) < sizeof(acWebDebug) ) sprintf(acWebDebug, "%s#%i: %s\n", acWebDebug, u16WebMsgNo++, s);}
#else
 #define WEB_DEBUG_PRINT(s) ;
#endif



// ---------------------------------------------------------------
// User configuration area end
// ---------------------------------------------------------------


#include <ESP8266WiFi.h>

#if MQTT_SUPPORTED == 1
#include <PubSubClient.h>
#if MQTT_MAX_PACKET_SIZE < 512 
#error change MQTT_MAX_PACKET_SIZE to 512
// C:\Users\<user>\Documents\Arduino\libraries\pubsubclient-master\src\PubSubClient.h
#endif
#endif

#include "Growatt.h"
#include <ESP8266HTTPUpdateServer.h>

#include "index.h"

#if PINGER_SUPPORTED == 1
#include <Pinger.h>
#include <PingerResponse.h>
#endif

#include <stdio.h>
#include <stdlib.h>

#define LED_GN 0  // GPIO0
#define LED_RT 2  // GPIO2
#define LED_BL 16 // GPIO16

#define BUTTON 0 // GPIO0

#define NUM_OF_RETRIES 5
char u8RetryCounter = NUM_OF_RETRIES;

long lAccumulatedEnergy = 0;

const char* update_path = "/firmware";
uint16_t u16PacketCnt = 0;
#if PINGER_SUPPORTED == 1
Pinger pinger;
#endif

WiFiClient   espClient;
#if MQTT_SUPPORTED == 1 
PubSubClient MqttClient(espClient);
#endif
Growatt      Inverter;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;

char JsonString[MQTT_MAX_PACKET_SIZE] = "{\"Status\": \"Disconnected\" }";

// -------------------------------------------------------
// Check the WiFi status and reconnect if necessary
// -------------------------------------------------------
void WiFi_Reconnect()
{
  uint16_t cnt = 0;

  if( WiFi.status() != WL_CONNECTED )
  {
    digitalWrite(LED_GN, 0);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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

    #if MQTT_SUPPORTED == 1 
    MqttClient.setServer(MQTT_SERVER, 1883);
    #endif
    
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
  if( Inverter.GetWiFiStickType() == ShineWiFi_S )
    WEB_DEBUG_PRINT("ShineWiFi-S (Serial) found")
  else if( Inverter.GetWiFiStickType() == ShineWiFi_X )
    WEB_DEBUG_PRINT("ShineWiFi-X (USB) found")
  else
    WEB_DEBUG_PRINT("Error: Undef. Stick")
  #endif
}

// -------------------------------------------------------
// Check the Mqtt status and reconnect if necessary
// -------------------------------------------------------
#if MQTT_SUPPORTED == 1 
void MqttReconnect() 
{
  // Loop until we're reconnected
  while (!MqttClient.connected()) 
  {
    if( WiFi.status() != WL_CONNECTED )
      break;
    
    #if ENABLE_DEBUG_OUTPUT == 1
    Serial.print("Attempting MQTT connection...");
    #endif
    
    // Attempt to connect with last will
    if (MqttClient.connect("Growatt", MQTT_TOPIC, 1, 1, "{\"Status\": \"Disconnected\" }"))
    {
      #if ENABLE_DEBUG_OUTPUT == 1
      Serial.println("connected");
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
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
#endif

// -------------------------------------------------------
// Will be executed once after power on
// -------------------------------------------------------
void setup()
{
  pinMode(LED_GN, OUTPUT);
  pinMode(LED_RT, OUTPUT);
  pinMode(LED_BL, OUTPUT);

  WEB_DEBUG_PRINT("Setup()")
  
  WiFi.hostname(HOSTNAME);
  while (WiFi.status() != WL_CONNECTED)
    WiFi_Reconnect();

  httpServer.on("/status", SendJsonSite);
  httpServer.on("/postCommunicationModbus", SendPostSite);
  httpServer.on("/postCommunicationModbus_p", HTTP_POST, handlePostData);
  httpServer.on("/setAccumulatedEnergy", HTTP_POST, vSetAccumulatedEnergy);
  httpServer.on("/", MainPage);
  #if ENABLE_WEB_DEBUG == 1
  httpServer.on("/debug", SendDebug);
  #endif

  InverterReconnect();

  httpUpdater.setup(&httpServer, update_path, UPDATE_USER, UPDATE_PASSWORD);
  httpServer.begin();
}

void SendJsonSite(void)
{
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
                  "<input type=\"text\" name=\"reg\" placeholder=\"RegDec\"></br>"
                  "<input type=\"text\" name=\"val\" placeholder=\"ValueDec(16Bit)\"></br>"
                  "<input type=\"checkbox\" id=\"rd\" name=\"rd\" value=\"Rd\" checked>"
                  "<label for=\"rd\"> Read</label></br>"
                  "<input type=\"submit\" value=\"Go\">"
                  "</form>");
}

void vSetAccumulatedEnergy()
{
  if (httpServer.hasArg("AcE"))
  {
    // only react if AcE is transmitted
    char * msg;
    msg = JsonString;

    if (lAccumulatedEnergy <= 0)
    {
      lAccumulatedEnergy = httpServer.arg("AcE").toInt() * 3600;
      sprintf(msg, "Setting accumulated value to %d", httpServer.arg("AcE").toInt());
    }
    else
    {
      sprintf(msg, "Error: AccumulatedEnergy was not Zero or lower. Set to 0 first.");
    }

    if (httpServer.arg("AcE").toInt() == 0)
    {
      lAccumulatedEnergy = -1000 * 3600;
      sprintf(msg, "Prepared to set AcE. You can change it as long as it is negative.");
    }

    httpServer.send(200, "text/plain", msg);
  }
  else
  {
    httpServer.send(400, "text/plain", "400: Invalid Request"); // The request is invalid, so send HTTP status 400
  }
}

void handlePostData()
{
  char * msg;
  uint16_t u16Tmp;

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
    if (httpServer.arg("rd") == "Rd")
    {
      if (Inverter.ReadHoldingReg(httpServer.arg("reg").toInt(), & u16Tmp))
      {
        sprintf(msg, "Read register %d with value %d", httpServer.arg("reg").toInt(), u16Tmp);
      } 
      else 
      {
        sprintf(msg, "Read register %d impossible - not connected?", httpServer.arg("reg").toInt());
      }
    }
    else
    {
      if (Inverter.WriteHoldingReg(httpServer.arg("reg").toInt(), httpServer.arg("val").toInt()))
        sprintf(msg, "Wrote Register %d to a value of %d!", httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
      else
        sprintf(msg, "Did not write Register %d to a value of %d - fault!", httpServer.arg("reg").toInt(), httpServer.arg("val").toInt());
    }
    httpServer.send(200, "text/plain", msg);
    return;
  }
}

// -------------------------------------------------------
// Main loop
// -------------------------------------------------------
long Timer500ms = 0;
long Timer5s = 0;
long Timer2m = 0;

void loop()
{
  long now = millis();
  long lTemp;
  char readoutSucceeded;
  
  WiFi_Reconnect();
  
  #if MQTT_SUPPORTED == 1
  MqttReconnect();
  MqttClient.loop();
  #endif
  
  httpServer.handleClient();

  // Toggle green LED with 1 Hz (alive)
  // ------------------------------------------------------------
  if( (now - Timer500ms) > 500 )
  {   
    if( WiFi.status() == WL_CONNECTED )
      digitalWrite(LED_GN, !digitalRead(LED_GN));
    else
      digitalWrite(LED_GN, 0);
    
    Timer500ms = now;
  }

  // InverterReconnect() takes a long time --> wifi will crash
  // Do it only every two minutes
  if( (now - Timer2m) > (1000 * 60 * 2) )
  {
    if( Inverter.GetWiFiStickType() == Undef_stick )
      InverterReconnect();
    Timer2m = now;
  }

    // Read Inverter every 5 s
  // ------------------------------------------------------------
  if( (now - Timer5s) > 5000)
  {
    #if MQTT_SUPPORTED == 1
    if (MqttClient.connected() && (WiFi.status() == WL_CONNECTED) && (Inverter.GetWiFiStickType()) )
    #else
    if( 1 )
    #endif
    {
      readoutSucceeded = 0;
      while ((u8RetryCounter) && !(readoutSucceeded))
      {
        #if SIMULATE_INVERTER == 1
        if( 1 ) // do it always
        #else
        if( Inverter.UpdateData() ) // get new data from inverter
        #endif
        {
          WEB_DEBUG_PRINT("UpdateData() successful")
          u16PacketCnt++;
          u8RetryCounter = NUM_OF_RETRIES;
          CreateJson(JsonString);
    
          #if MQTT_SUPPORTED == 1 
          MqttClient.publish(MQTT_TOPIC, JsonString, true);
          #endif
      

          // if we got data, calculate the accumulated energy
          lTemp = (now - Timer5s) * Inverter.GetAcPower();      // we now get an increment in milliWattSeconds
          lTemp /= 1000;                                        // WattSeconds
          lAccumulatedEnergy += lTemp;                          // WattSeconds
          
          digitalWrite(LED_RT, 0); // clear red led if everything is ok
          // leave while-loop
          readoutSucceeded = 1;
        }
        else
        {
          WEB_DEBUG_PRINT("UpdateData() NOT successful")
          if(u8RetryCounter)
          {
            u8RetryCounter--;
          }
          else
          {
            WEB_DEBUG_PRINT("Retry counter\n")
            sprintf(JsonString, "{\"Status\": \"Disconnected\" }");
            #if MQTT_SUPPORTED == 1 
            MqttClient.publish(MQTT_TOPIC, JsonString, true);
            #endif
            digitalWrite(LED_RT, 1); // set red led in case of error
          }
        }
      }
    }

    #if MQTT_SUPPORTED == 1
    if (!MqttClient.connected())
      digitalWrite(LED_RT, 1);
    else
      digitalWrite(LED_RT, 0);
    #endif
    
    #if PINGER_SUPPORTED == 1
    //frequently check if gateway is reachable
    if (pinger.Ping(GATEWAY_IP) == false) 
      WiFi.disconnect();
    #endif

    Timer5s = now;
  }
}

void CreateJson(char *Buffer)
{
  Buffer[0] = 0; // Terminate first byte

#if SIMULATE_INVERTER != 1
  sprintf(Buffer, "{\r\n");
  switch( Inverter.GetStatus() )
  {
    case GwStatusWaiting:
      sprintf(Buffer, "%s  \"Status\": \"Waiting\",\r\n", Buffer);
      break;
    case GwStatusNormal: 
      sprintf(Buffer, "%s  \"Status\": \"Normal\",\r\n", Buffer);
      break;
    case GwStatusFault:
      sprintf(Buffer, "%s  \"Status\": \"Fault\",\r\n", Buffer);
      break;
    default:
      sprintf(Buffer, "%s  \"Status\": \"%d\",\r\n", Buffer, Inverter.GetStatus());
  }
  
  sprintf(Buffer, "%s  \"DcVoltage\": %.1f,\r\n",       Buffer, Inverter.GetDcVoltage());
  sprintf(Buffer, "%s  \"AcFreq\": %.3f,\r\n",          Buffer, Inverter.GetAcFrequency());
  sprintf(Buffer, "%s  \"AcVoltage\": %.1f,\r\n",       Buffer, Inverter.GetAcVoltage());
  sprintf(Buffer, "%s  \"AcPower\": %.1f,\r\n",         Buffer, Inverter.GetAcPower());
  sprintf(Buffer, "%s  \"EnergyToday\": %.1f,\r\n",     Buffer, Inverter.GetEnergyToday());
  sprintf(Buffer, "%s  \"EnergyTotal\": %.1f,\r\n",     Buffer, Inverter.GetEnergyTotal());
  sprintf(Buffer, "%s  \"OperatingTime\": %u,\r\n",     Buffer, Inverter.GetOperatingTime());
  sprintf(Buffer, "%s  \"Temperature\": %.1f,\r\n",     Buffer, Inverter.GetInverterTemperature());
  sprintf(Buffer, "%s  \"AccumulatedEnergy\": %d,\r\n", Buffer, lAccumulatedEnergy / 3600);
  sprintf(Buffer, "%s  \"Cnt\": %u\r\n",                Buffer, u16PacketCnt);
  sprintf(Buffer, "%s}\r\n", Buffer);
#else
  #warning simulating the inverter
  sprintf(Buffer, "{\r\n");
  sprintf(Buffer, "%s  \"Status\": \"Normal\",\r\n",     Buffer);
  sprintf(Buffer, "%s  \"DcVoltage\": 70.5,\r\n",        Buffer);
  sprintf(Buffer, "%s  \"AcFreq\": 50.00,\r\n",          Buffer);
  sprintf(Buffer, "%s  \"AcVoltage\": 230.0,\r\n",       Buffer);
  sprintf(Buffer, "%s  \"AcPower\": 0.00,\r\n",          Buffer);
  sprintf(Buffer, "%s  \"EnergyToday\": 0.3,\r\n",       Buffer);
  sprintf(Buffer, "%s  \"EnergyTotal\": 49.1,\r\n",      Buffer);
  sprintf(Buffer, "%s  \"OperatingTime\": 123456,\r\n",  Buffer);
  sprintf(Buffer, "%s  \"Temperature\": 21.12,\r\n",     Buffer);
  sprintf(Buffer, "%s  \"AccumulatedEnergy\": 320,\r\n", Buffer);
  sprintf(Buffer, "%s  \"Cnt\": %u\r\n",                 Buffer, u16PacketCnt);
  sprintf(Buffer, "%s}", Buffer);
#endif // SIMULATE_INVERTER
}
