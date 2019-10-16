# Growatt_ShineWiFi-S
Firmware replacement for Growatt ShineWiFi-S

This arduino sketch will replace the original firmware of the Growatt ShineWiFi-S Stick.
The data received from the inverter will be transmitted by MQTT to a server of your choice.

The manufacturer has been so kindly to bring out every necessary connection for reprograming. (s. ShineWiFi-S_PCB_PinOut.png)

I added a 1k resistor between the output of the SW1 and GPIO0. So it is possible to put the device into boot mode on start up.

Take notice on the PCB labeling of the UART pins. The pin labeled with "Tx" is the Rx pin of the microcontroller. So Tx has to be connected with Tx of your USB to serail converter. (Same for Rx)

For ducumentation on the Modbus interface search for "Growatt PV Inverter Modbus RS485 RTU Protocol" on Google.

The PCB is also populated with 8 MB of serial flash, and a NXP 8563T real time clock. Both is not used in this project.

Some keywords:

ES8266, ESP-07S, Growatt, Arduino, MQTT, Modbus

Thanks to Jethro Kairys for his work on the Modbus interface
https://github.com/jkairys/growatt-esp8266
