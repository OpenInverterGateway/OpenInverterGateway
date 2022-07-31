# Growatt_ShineWiFi
Firmware replacement for Growatt ShineWiFi-S (serial) and ShineWiFi-X (USB)

This arduino sketch will replace the original firmware of the Growatt ShineWiFi stick.

This project is based on Jethro Kairys work on the Modbus interface
https://github.com/jkairys/growatt-esp8266


## Features
Implemented Features:
* Built-in simple Webserver
* The inverter is queried using Modbus Protocol
* The data received will be transmitted by MQTT to a server of your choice.
* The data received is also provied as JSON 
* Show a simple live graph visualization  (`http://<ip>`) with help from highcharts.com
* It supports convenient OTA firmware update (`http://<ip>/firmware`)
* It supports basic access to arbitrary modbus data
* It tries to autodected which protocol version to use
* Wifi manager with own access point for initial configuration of Wifi and MQTT server (IP: 192.168.4.1, SSID: GrowattConfig, Pass: growsolar)

Not supported:
* It does not make use the RTC or SPI Flash of these boards..
* It does not communicate to Growatt Cloud at all.

Potential features not implemented yet:
* Extend to support 3-Phase-AC inverters
* Extend to support additional PV strings
* Since it is all Modbus, other imvertery could be added.


## Supported devices
* ShineWifi-S with Growatt 1000S connected to serial (Modbus over RS232 with level shifter)
* ShineWifi-X with Growatt 600TL-X connectd to USB (USB-Serial Chip from Exar)
* Wemos-D1 with Growatt 600TL-X connected to USB (USB-Serial Chip: CH340)
* ShineWifi-T (untested, please give feedback)

I expect that almost any ESP8266 module with an USB-Serial chip will work out of the box on inverters with USB port.
I expect that almost any ESP8266 module with added 9-Pin Serial port and level shifter will work with little soldering.

See the short decriptions to the devices in den directories with their pictures in /IMG/*


## Modbus Protocol Versions
The documentation from Growatt on the Modbus interface is avaliable, search for "Growatt PV Inverter Modbus RS485 RTU Protocol" on Google.

The older inverters apparently use Protocol v3.05 from year 2013.
The newer inverters apparently use protocol v1.05 from year 2018.
The newer protocol allows for additional inverter functionality, which is not implemented in this firmware (yet).


## JSON Format Data
For IoT applications the raw data can now read in JSON format (application/json) by calling `http://<ip>/status`

example:

    {
      "Status": "Normal",
      "DcVoltage": 114.1,
      "AcFreq": 50.000,
      "AcVoltage": 239.5,
      "AcPower": 20.6,
      "EnergyToday": 0.2,
      "EnergyTotal": 48.3,
      "OperatingTime": 2821777,
      "Temperature": 12.1,
      "AccumulatedEnergy": 320,
      "Cnt": 333
    }


Some keywords:

ESP8266, ESP-07S, Growatt 1000S, Growatt 600TL, ShineWifi, Arduino, MQTT, JSON, Modbus, Rest



## 2020-01-18 Update

* For IoT applications the raw data can now read in JSON format (application/json) by calling `http://<ip>/status`

example:

    {
      "Status": "Normal",
      "DcVoltage": 114.1,
      "AcFreq": 50.000,
      "AcVoltage": 239.5,
      "AcPower": 20.6,
      "EnergyToday": 0.2,
      "EnergyTotal": 48.3,
      "OperatingTime": 2821777,
      "Temperature": 12.1,
      "AccumulatedEnergy": 320,
      "Cnt": 333
    }


* Firmware can be updated over the Webserver (`http://<ip>/firmware`)
  
* A status website with live graph can be found under `http://<ip>`

## 2020-10-22 Update
* Added support for ShineWiFi-X (USB). The software will automatically detect on which kind of stick it is running.

* Registers of the inverter can be read or written with an web interface (`http://<ip>/postCommunicationModbus`)

* The total energy is only stored in increments of 0.1 kWh in the inverter. For a smal inverter, this is a large step, especially during winter. 
If the total energy is 0.199 kWh before sunset, the totoal enrgy will be reset to 0.1 kWh next morning. For this reason the "AccumulatedEnergy" field has been implemented. It returns the energy in Wh. It will be zero after every powercycle, but can be set to the value of the previous day over a http request (`http://<ip>/setAccumulatedEnergy`)

* MQTT can be turned off

* The stick can ping an known IP. If there is no answer, the stick will try to reset the WiFi connection. (Disabled by default)

## 2021-10-27 Update
* The automatic detection of the inverter can fail after sunrise. The stick powers up several minutes before the inverter. The detection will only take place directly after power on and will fail because the inverter is not running yet. If the stick can not identify the inverter, it will redo the detection every 2 minutes.
* Debug messages can be read from `<ip>/debug`

* It will automatically detect the stick (USB or serial) and will use the correct baudrate and register set. 
* Added counter for accumulated energy
* Interface for register read/write over web interface added
* MQTT can be turned off by compiler switch
* WiFi connection can be reset if a known IP can not be pinged (optional)

@BeoQ Thanks for your investigations

     

## 2022-02-18 Update

*    Graph will display the data with the timezone of the host pc (UTC was used before)
*    Moved the source to a folder which is named like the arduino project, because arduino IDE will otherwise not open the project


## 2022-05-24 Update

* Export already-implemented DcPower and DcInputCurrent values
* Extend RW-Modbus Webpage to also read Input Registers
* Prepare sGrowattData for additional Registers

## 2022-07-31 Update
* Wifi manager added. Thanks to @roel80

