# Growatt_ShineWiFi
Firmware replacement for Growatt ShineWiFi-S (serial), ShineWiFi-X (USB) or custom build sticks (ESP8266/ESP32).

# How to install
* Checkout this repo
* Setup the IDE of your choice
    * For the original Arduino IDE follow the instruction in the main ino [file](https://github.com/otti/Growatt_ShineWiFi-S/blob/master/SRC/ShineWiFi-ModBus/ShineWiFi-ModBus.ino)
    * For platformio just open the project folder and choose the correct env for your hardware
* Rename and adapt [Config.h.example](https://github.com/otti/Growatt_ShineWiFi-S/blob/master/SRC/ShineWiFi-ModBus/Config.h.example) to Config.h with your compile time settings
* Flash to an esp32/esp8266
* For detailed flashing instructions see https://github.com/otti/Growatt_ShineWiFi-S/blob/master/Doc/
* Connect to the setup wifi called GrowattConfig (PW: growsolar) and configure the firmware via the webinterface at http://192.168.4.1
* If you need to reconfigure the stick later on you have to either press the ap button (configured in Config.h) or reset the stick twice within 10sec

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
* Currently Growatt v1.20, v1.24 and v3.05 protocols are implemented and can be easily extended/changed to fit anyone's needs

Not supported:
* It does not make use the RTC or SPI Flash of these boards..
* It does not communicate to Growatt Cloud at all.

Potential features not implemented yet:
* Extend to support 3-Phase-AC inverters *done*
* Extend to support additional PV strings *done*
* Since it is all Modbus, other imverters could be added.

## Supported sticks/microcontrollers
* ShineWifi-S with a Growatt Inverter connected to serial (Modbus over RS232 with level shifter)
* ShineWifi-X with a Growatt Inverter connectd to USB (USB-Serial Chip from Exar)
* Wemos-D1 with a Growatt Inverter connected to USB (USB-Serial Chip: CH340)
* NODEMCU V1 (ESP8266) with Growatt MIC 2000TL connected to to USB (USB-Serial Chip: CH340)
* ShineWifi-T (untested, please give feedback)

I tested several ESP8266-boards with builtin USB-Serial converters so far only boards with CH340 do work (CP21XX and FTDI chips do not work). As
an ESP32 board the lolin32 is a good hardware choice. I guess almost all ESP8266 modules with added 9-Pin Serial port and level shifter will work with little soldering.

See the short descriptions to the devices in den directories with their pictures in /IMG/*

## Supported Interters
* Growatt 1000-3000S 
* Growatt MIC 600-3300TL-X (Protocol 124 via USB/Protocol 120 via Serial)
* Growatt MID 3-25ktl3-x (Protocol 124 via USB)
* Growatt SPH 4000-10000KTL3 BH (Protocol 124)
* And others ....

## Modbus Protocol Versions
The documentation from Growatt on the Modbus interface is avaliable, search for "Growatt PV Inverter Modbus RS485 RTU Protocol" on Google.

The older inverters apparently use Protocol v3.05 from year 2013.
The newer inverters apparently use protocol v1.05 from year 2018.
There is also a new protocol version v1.24 from 2020. (used with SPH4-10KTL3 BH-UP inverter)


## JSON Format Data
For IoT applications the raw data can now read in JSON format (application/json) by calling `http://<ip>/status`

example:

    {
      "input": {
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
      },
      "holding": {}
    }


Some keywords:

ESP8266, ESP-07S, Growatt 1000S, Growatt 600TL, ShineWifi, Arduino, MQTT, JSON, Modbus, Rest

## Acknowledgements

This arduino sketch will replace the original firmware of the Growatt ShineWiFi stick.

This project is based on Jethro Kairys work on the Modbus interface
https://github.com/jkairys/growatt-esp8266

## 2020-01-18 Update

* For IoT applications the raw data can now read in JSON format (application/json) by calling `http://<ip>/status`

example:

    {
        "InverterStatus": "Normal",
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
* Graph will display the data with the timezone of the host pc (UTC was used before)
* Moved the source to a folder which is named like the arduino project, because arduino IDE will otherwise not open the project


## 2022-05-24 Update
* Export already-implemented DcPower and DcInputCurrent values
* Extend RW-Modbus Webpage to also read Input Registers
* Prepare sGrowattData for additional Registers

## 2022-07-31 Update
* Wifi manager added. Thanks to @roel80

## 2022-08-04
* Added support for platform-io (crasu, zinserjan)
* Added support for ESP32 (crasu)

## 2022-08-24 Update
* Redesigned the Growatt class to manage various protocols. (v3.05, v1.20 and v1.24 implemented)
* The new protocol definition allows user to define what modbus register should be read and which ones should be:
	* exported in JSON by calling `http://<ip>/status`
	* displayed on the UI `http://<ip>`
	* added to the graph in the UI
* MQTT_MAX_PACKET_SIZE doesn't need to be updated manually in the library. Instead the `PubSubClient::setBufferSize`has been used to do this dynamically.
* New `http://<ip>/uistatus` has been created to provide data for the UI
* UI changes:
	* Ui is generated dynamically based on the JSON provided.
	* Graph is now able to plot multiple values

