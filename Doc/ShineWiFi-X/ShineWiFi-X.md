# Description of Hardware

Collection of Information around this Hardware

## ShineWifi-X

BeoQ, December 2021


### Physical build

The stick hast a Standard USB-connector (Type-A, male) that can be secured to the inverter by some locking ring.  
The locking is a moveable part of the plastic connector-side housing of the Stick.

On the inverter-side, any USB cable should fit.  
On the stick-side, it won't fit to a standard computer due to the locking ring, but USB extension cords should work.

To open the stick's housing, push in the two plastic retention tongues, and pull off the rear housing from the connector side of the housing.


### Electrical build

The PCB is equipped with the following major functional parts

* USB-to-Serial Converter
* Linear voltage regulator
* ESP8266 in a ESP07 package
* RTC on I2C-bus with CR1220 backup battery
* Flash on SPI-bus
* Three status LEDs
* One pushbutton
* WiFi Antenna wire on IPEX connector


The PCB layout supports two alternative types of USB-to-serial converter:
Exar/MaxLinear XR21V1410 or a CH340, or a direct connection to the USB-Pins, selectable by coosing which 0R resistors are soldered.

There is an unpopulated connector CN1
1: 3V3  
2:	GND  
3:	RX0  
4:	TX0  
5:	TX1  
6:	GPIO0  
7:	GND  


The LEDs are driven toward Ground, i.e. a Logical HIGH turns them on.

The pushbutton is connected to the Analog Input of the ESP through a voltage divider pull-up to 3.3V.


Reverse Engineering of the connections to the ESP-07 module:

ESP-07S Module (16Pin) on ShineWifi-X:



|ESP Pin No|ESP Pin Func.|Stick      |
|---|------|-------------------------|
| 1 | RST	 |	R19 	Pullup             |
| 2 | ADC	 |	R1/R8 	Switch to GND    |
| 3 | EN		 |	R16 	Pullup             |
| 4 | IO16	|	R11  	LED3              |
| 5 | IO14	|	SPI_CLK	U5_6	(to Flash) |
| 6 | IO12	|	SPI_DO	U5_2	(to Flash)  |
| 7 | IO13	|	SPI_DI	U5_5	(from Flash)|
| 8 | VCC	 |	+3.3V                   |
| 9 | GND	 |	Ground                  |
|10 | IO15	|	SPI_CS                  |
|11 | IO02	|	TX1	R9	LED2             |
|12 | IO00	|	GPIO0	R6	LED1           |
|13 | IO04	|	I2C_SCL	R7	U1_6         |
|14 | IO05	|	I2C_SDA	R10	U1_5        |
|15 | RXD0	|	RX0	R30	UART_TxD        |
|16 | TXD0	|	TX0	R29	UART_RxD        |


### Modbus Communication

My ShineWifi-X stick was delivered along with a Growatt 600TL-X.  
In this combination, the inverter is the USB host that activates the USB-to-serial chip.  
Then, the ShineWiFi-X is the Modbus master, while the inverter responds to Modbus address #1 at 115200 Baud, 8N1

Modbus Protocol description v1.05 dated 2018 seems to be applicable.

It uses Modbus command 0x03 (Read Input Registers) for all the volatile information (i.e voltages, power, ...)  
It uses Modbus command 0x04 (Read Holding Registers) for static information (i.e. capabilities, firmware version, ...)

Note: at the inverter, this modbus interface at 115200Bd seems to be simultaneously usable to the RS485-based Modbus interface of the communication connector directly at the inverter at 9600Bd.


### Loading Firmware

Initial loading of Firmware to the stick is very easy:

Remove the stick's PCB from the housing.
Temporarily  connect the header's Pins GPIO0 and GND while powering-on (plugging in) the stick.
Remember to install the USB driver for the USB-to-Serial chip if needed by your computer's OS.
Upload of the compiled binary of the firmware. (using arduino-ide, esptool or avrdude, whatever you prefer)

I have used a male dupont-wire pushed in place while plugging into my USB-extension cord.


Updating an installed firmware is very easy using OTA the builtin config webserver (192.168.4.1):

Use:  http://&lt;config ip&gt;/ and click on update.
