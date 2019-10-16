/*
Board: Generic ESP8266 Module
Flash Mode: DIO
Flash Freq: 40 MHz
Upload Using: Serial
CPU Freq: 80 MHz
Flash Size: 512k (64k SPIFFS)
UploadSpeed: 115200

Thanks to Jethro Kairys
https://github.com/jkairys/growatt-esp8266


2019-10-16
*/
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "Growatt.h"


#include <stdio.h>
#include <stdlib.h>

#define LED_GN 0  // GPIO0
#define LED_RT 2  // GPIO2
#define LED_BL 16 // GPIO16


// Data of the Wifi access point
const char* ssid        = "YOUR_SSID";
const char* password    = "YOUR_PASSWORD";
#define HOSTNAME          "Growatt"

// IP of the MQTT-Server
const char* mqtt_server = "192.168.0.38";


WiFiClient   espClient;
PubSubClient MqttClient(espClient);
Growatt      Inverter;

// -------------------------------------------------------
// Check the WiFi status and reconnect if necessary
// -------------------------------------------------------
void WiFi_Reconnect()
{
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
    Serial.print("Attempting MQTT connection...");
    
    // Attempt to connect with last will
    if (MqttClient.connect("Growatt", "LS111/Solar/Growatt1kWp/Status", 1, 1, "Disconnected")) {
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

  Serial.begin(9600);

  WiFi.hostname(HOSTNAME);
  WiFi_Reconnect();
  Inverter.begin(Serial);
}

// -------------------------------------------------------
// Send the current inverter status by mqtt
// -------------------------------------------------------
void PublishStatus(eGrowattStatus status)
{
  char buffer[16];
  
  String str = "unknown";
  switch(status){
    case GwStatusWaiting:
      str = "waiting";
      break;
    case GwStatusNormal:
      str = "normal";
      break;
    case GwStatusFault: 
      str = "fault";
      break;
  }
  str.toCharArray(buffer, 16);
  MqttClient.publish("LS111/Solar/Growatt1kWp/Status", buffer, true);
  
}

// -------------------------------------------------------
// Send a float value by mqtt
// Para: topic
// Para: value to send
// Para: number of decimal places
// -------------------------------------------------------
void PublishFloat(char * topic, float f, uint8_t precision)
{
  String value_str = String(f, precision);
  char value_char[32] = "";
  value_str.toCharArray(value_char, 40);

  String topic_str = "LS111/Solar/Growatt1kWp/" + String(topic);
  char topic_char[128] = "";
  topic_str.toCharArray(topic_char, 128);
  
  MqttClient.publish(topic_char, value_char, true);
}

// -------------------------------------------------------
// Send an integer value by mqtt
// Para: topic
// Para: value to send
// -------------------------------------------------------
void PublishInteger(char * topic, uint32_t i)
{
  String value_str = String(i);
  char value_char[32] = "";
  value_str.toCharArray(value_char, 40);

  String topic_str = "LS111/Solar/Growatt1kWp/" + String(topic);
  char topic_char[128] = "";
  topic_str.toCharArray(topic_char, 128);
  
  MqttClient.publish(topic_char, value_char, true);
}


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
    if( Inverter.UpdateData() ) // get new data from inverter
    {

      PublishStatus(                  Inverter.GetStatus());
      PublishFloat("DcVoltage",       Inverter.GetDcVoltage(),           1);
      PublishFloat("AcFreq",          Inverter.GetAcFrequency(),         2);
      PublishFloat("AcVoltage",       Inverter.GetAcVoltage(),           1);
      PublishFloat("AcPower",         Inverter.GetAcPower(),             1);
      PublishFloat("EnergyToday",     Inverter.GetEnergyToday(),         1);
      PublishFloat("EnergyTotal",     Inverter.GetEnergyTotal(),         1);
      PublishInteger("OperatingTime", Inverter.GetOperatingTime());
      PublishFloat("Temperature",     Inverter.GetInverterTemperature(), 1);

      digitalWrite(LED_BL, 0); // clear blue led if everything is ok
    }
    else
    {
      digitalWrite(LED_BL, 1); // set blue led in case of error
    }
    
    Timer10s = now;
  }

}
