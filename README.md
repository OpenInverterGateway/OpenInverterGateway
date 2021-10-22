# Growatt_ShineWiFi
Firmware replacement for Growatt ShineWiFi-S (serial) and ShineWiFi-X (USB)

This arduino sketch will replace the original firmware of the Growatt ShineWiFi stick.
The data received from the inverter will be transmitted by MQTT to a server of your choice.

The manufacturer has been so kindly to bring out every necessary connection for reprograming. (s. /IMG/ShineWiFi-S/ShineWiFi-S_PCB_PinOut.png)

I added a 1k resistor between the output of the SW1 and GPIO0. So it is possible to put the device into boot mode on start up.

Take notice on the PCB labeling of the UART pins. The pin labeled with "Tx" is the Rx pin of the microcontroller. So Tx has to be connected with Tx of your USB to serial converter. (Same for Rx)

For documentation on the Modbus interface, search for "Growatt PV Inverter Modbus RS485 RTU Protocol" on Google.

The PCB is also populated with 8 MB of serial flash, and a NXP 8563T real time clock. Both is not used in this project.

Some keywords:

ES8266, ESP-07S, Growatt 1000S, Arduino, MQTT, JSON, Modbus, Rest

Thanks to Jethro Kairys for his work on the Modbus interface
https://github.com/jkairys/growatt-esp8266

## Tested devices
* ShineWiFi-S with Growatt 1000S
* ShineWiFi-X

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