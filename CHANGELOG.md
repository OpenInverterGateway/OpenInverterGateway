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

## 2023-01-20 Update
* Added tls support for esp32
