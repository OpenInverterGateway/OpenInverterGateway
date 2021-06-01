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
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Growatt.h"
#include <ESP8266HTTPUpdateServer.h>

#include "index.h"


#include <stdio.h>
#include <stdlib.h>

#define LED_GN 0  // GPIO0
#define LED_RT 2  // GPIO2
#define LED_BL 16 // GPIO16

#define BUTTON 0 // GPIO0

// Data of the Wifi access point
const char* ssid        = "<your_ssid>";
const char* password    = "<your_wifi_password>";
#define HOSTNAME          "Growatt"

const char* update_path = "/firmware";
const char* update_username = "<user_for_update>";
const char* update_password = "<password_for_update>";


const char* mqtt_server = "<mqtt_server_ip";


WiFiClient   espClient;
PubSubClient MqttClient(espClient);
Growatt      Inverter;
ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;


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
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
      delay(200);
      Serial.print("x");
      digitalWrite(LED_RT, !digitalRead(LED_RT)); // toggle red led on WiFi (re)connect
    }

    Serial.println("");
    WiFi.printDiag(Serial);
    Serial.print("local IP:");
    Serial.println(WiFi.localIP());
    Serial.print("Hostname: ");
    Serial.println(HOSTNAME);

    MqttClient.setServer(mqtt_server, 1883);
    
    digitalWrite(LED_RT, 0);
  }
}


// -------------------------------------------------------
// Check the Mqtt status and reconnect if necessary
// -------------------------------------------------------
void MqttReconnect() 
{
  // Loop until we're reconnected
  while (!MqttClient.connected()) 
  {
    if( WiFi.status() != WL_CONNECTED )
      break;
    
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect with last will
    if (MqttClient.connect("Growatt", "LS111/Solar/Growatt1kWp", 1, 1, "{\"Status\": \"Disconnected\" }")) {
      Serial.println("connected");
    } 
    else
    {
      Serial.print("failed, rc=");
      Serial.print(MqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


// -------------------------------------------------------
// Will be executed once after power on
// -------------------------------------------------------
void setup()
{
  pinMode(LED_GN, OUTPUT);
  pinMode(LED_RT, OUTPUT);
  pinMode(LED_BL, OUTPUT);

  httpServer.on("/status", SendJsonSite);
  httpServer.on("/", MainPage);

  Serial.begin(9600); // Baudrate of Growatt

  WiFi.hostname(HOSTNAME);
  WiFi_Reconnect();
  Inverter.begin(Serial);

  httpUpdater.setup(&httpServer, update_path, update_username, update_password);
  httpServer.begin();
}

char MqttPayload[512] = "{\"Status\": \"Disconnected\" }";
#if MQTT_MAX_PACKET_SIZE < 512 
#error change MQTT_MAX_PACKET_SIZE to 512
#endif
uint16_t u16PacketCnt = 0;

void SendJsonSite(void)
{
  httpServer.send(200, "application/json", MqttPayload);
}

void MainPage(void)
{
  httpServer.send(200, "text/html", MAIN_page);
}

//#define SIMULATE 1

// -------------------------------------------------------
// Main loop
// -------------------------------------------------------
long Timer1s = 0;
long Timer10s = 0;

void loop()
{

  long now = millis();
  
  WiFi_Reconnect();
  MqttReconnect();
  
  httpServer.handleClient();
  
  MqttClient.loop();

  // Toggle green LED with 1 Hz (alive)
  // ------------------------------------------------------------
  if (now - Timer1s > 500)
  {
    if( WiFi.status() == WL_CONNECTED )
      digitalWrite(LED_GN, !digitalRead(LED_GN));
    else
      digitalWrite(LED_GN, 0);
    
    Timer1s = now;
  }


    // Read Inverter every 10 s
  // ------------------------------------------------------------
  if (now - Timer10s > 10000)
  {
    if( MqttClient.connected() && (WiFi.status() == WL_CONNECTED) )
    {
      #ifndef SIMULATE
      if( Inverter.UpdateData() ) // get new data from inverter
      #else
      if(1)
      #endif
      {
        u16PacketCnt++;
        CreateJson(MqttPayload);
  
        
        MqttClient.publish("LS111/Solar/Growatt1kWp", MqttPayload, true);
    
        digitalWrite(LED_BL, 0); // clear blue led if everything is ok
      }
      else
      {
        sprintf(MqttPayload, "{\"Status\": \"Disconnected\" }");
        MqttClient.publish("LS111/Solar/Growatt1kWp", MqttPayload, true);
        digitalWrite(LED_BL, 1); // set blue led in case of error
      }
    }

    Timer10s = now;
  }

}

void CreateJson(char *Buffer)
{
  
  Buffer[0] = 0; // Terminate first byte
  
#ifndef SIMULATE
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
  }
  sprintf(Buffer, "%s  \"DcVoltage\": %.1f,\r\n",     Buffer, Inverter.GetDcVoltage());
  sprintf(Buffer, "%s  \"AcFreq\": %.3f,\r\n",        Buffer, Inverter.GetAcFrequency());
  sprintf(Buffer, "%s  \"AcVoltage\": %.1f,\r\n",     Buffer, Inverter.GetAcVoltage());
  sprintf(Buffer, "%s  \"AcPower\": %.1f,\r\n",       Buffer, Inverter.GetAcPower());
  sprintf(Buffer, "%s  \"EnergyToday\": %.1f,\r\n",   Buffer, Inverter.GetEnergyToday());
  sprintf(Buffer, "%s  \"EnergyTotal\": %.1f,\r\n",   Buffer, Inverter.GetEnergyTotal());
  sprintf(Buffer, "%s  \"OperatingTime\": %u,\r\n",   Buffer, Inverter.GetOperatingTime());
  sprintf(Buffer, "%s  \"Temperature\": %.1f,\r\n",    Buffer, Inverter.GetInverterTemperature());
  sprintf(Buffer, "%s  \"Cnt\": %u\r\n",              Buffer, u16PacketCnt);
  sprintf(Buffer, "%s}\r\n", Buffer);
#else
  #warning simulating
  sprintf(Buffer, "{\r\n");
  sprintf(Buffer, "%s  \"Status\": \"Normal\",\r\n",    Buffer);
  sprintf(Buffer, "%s  \"DcVoltage\": 70.5,\r\n",       Buffer);
  sprintf(Buffer, "%s  \"AcFreq\": 50.00,\r\n",         Buffer);
  sprintf(Buffer, "%s  \"AcVoltage\": 230.0,\r\n",      Buffer);
  sprintf(Buffer, "%s  \"AcPower\": 0.00,\r\n",         Buffer);
  sprintf(Buffer, "%s  \"EnergyToday\": 0.3,\r\n",      Buffer);
  sprintf(Buffer, "%s  \"EnergyTotal\": 49.1,\r\n",     Buffer);
  sprintf(Buffer, "%s  \"OperatingTime\": 123456,\r\n", Buffer);
  sprintf(Buffer, "%s  \"Temperature\": 21.12,\r\n",     Buffer);
  sprintf(Buffer, "%s  \"Cnt\": %u\r\n",                Buffer, u16PacketCnt);
  sprintf(Buffer, "%s}", Buffer);
      
  #endif 
}
