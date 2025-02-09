# 16 Channel Current Meter to MQTT Gateway
This sketch runs on an ESP8266 and reads data from 16 Channel Current Measurement Module over RS485 Modbus and publishes the data over MQTT. This module is available Aliexpress on the following link: https://www.aliexpress.com/item/1005002090646451.html

**UPDATE - KNOWN ISSUES** if you have an issue that your ESP does not connect to the MQTT server, please scroll to the bottom to Known Issues.

![Setup](/img/20211120_181637.jpg)

This sketch publishes the 16 channel current information and some auxillary data. This unit is supposed to measure frequency as well, but for some reason it does not work for me. So I excluded that from the sketch. There is no voltage measurement, therefore to calculate power consumption you need a separate device to measure the exact voltage.
You can see below the data published to MQTT. Current measurement is 10 mA resolution.

![MQTT Data](/img/mqtt.png)

## Install
Download this repository and build and flash your ESP. You need an Arduino IDE with ESP8266 configuration added. You need a few additional libraries (see below). And before you build open the settings.h and set your credentials. I think they are self explanatory. Compile and upload.

## Libraries
The following libraries are used beside the "Standard" ESP8266 libraries:
- FastLED by Daniel Garcia
- ModbusMaster by Doc Walker
- ArduinoOTA
- SoftwareSerial

## PCB

I designed a custom PCB for this board (for a previous project), you can order if from here: https://www.pcbway.com/project/shareproject/ESP8266_Modbus_board.html

These are the components you need to build this. I purchased most of them a long time ago. I don't have the original listing, so I included a similar Aliexpress listing.
- Wemos D1 mini
- RS485 to TTL converter: https://www.aliexpress.com/item/1005001621798947.html

With these two components you can power the board using the micro USB on the Wemos D1 mini. My PCB includes components for mains supply:
- Hi-Link 5V power supply (https://www.aliexpress.com/item/1005001484531375.html)
- Fuseholder and 1A fuse
- Optional varistor
- 5.08mm pitch KF301-2P screw terminal block

And if you want to wire them yourself, these are the connections:
```
Wemod D1 mini (or other ESP)            TTL to RS485
D1                                      DE
D2                                      RE
D5                                      RO
D6                                      DI
5V                                      VCC
G(GND)                                  GND
```

## Videos

I explain everything about this project in this video:

[![Full review](https://img.youtube.com/vi/ZYC7fzFqOUs/0.jpg)](https://www.youtube.com/watch?v=ZYC7fzFqOUs)

## Current Reader Documentation

I copy here the documents that were on the original Aliexpress listing in case the listing disappers from Aliexpress.

![Dimensions](/img/dimensions.png)

![Modbus registers](/img/modbus_registers_01.png)

![Modbus registers](/img/modbus_registers_02.png)

![Wiring](/img/wiring.png)

### Below is the description of the Aliexpress listing

Type 1: 40A threading transformer
Type 2: 80A threading transformer
Type 3: 40A opening and closing transformer
Type 4: 120A opening and closing transformer

Parameters
- Power voltage: DC8~28V (with self-recovery insurance)
- Rated power consumption: 3W
- Ac current measurement error: Typical value ±2%50Hz, ±5% non-50hz
- Frequency measurement error: Typical value ± 2.5hz/frequency division coefficient
- AC current resolution: 0.04A (40A range)
- Data refresh frequency: >5Hz/ frequency division coefficient
- Communication interface: RS485 isolation band TVS protection
- Operating temperature: -20℃ ~ 50℃
- Working humidity: 5%~90% no condensation
- Operating frequency: [10Hz*5/ frequency division coefficient]~400Hz
- Communication baud rate: Default 9600, N, 8, 1
- Same network node: Maximum 32

Modbus-RTU communication protocol
Function code: 0x03, read multiple registers
Example: 01 03 00 00 00 38 44 18

Read 56 word data from device 01 address 00
Example: 01 06 00 03 00 02 F8 OB

Write data 0002H to device 01 address 03
Function code: 0x10, write multiple registers
Example: 01 10 00 03 00 01 02 00 02 27 A2
Write data 0002H to device 01 address 03

Matters needing attention
1. Only operators with certain electrical knowledge can conduct wiring and other operations on the product. If there is any unknown use, please consult our company.
2. Avoid using in high temperature, humidity and dust, and avoid direct sunlight.

## Known Issues
Apperantly there is an issue with the latest version of the ESP8266 board manager version and FastLED. The symptom is that the ESP cannot connect to the MQTT broker and get reset by the watchdog timer (WDT) all the time. I managed to install everything and compile a working ESP but downgrading FastLED to 3.3.3. These are my Arduino IDE settings, and at the moment everything is on the latest version, except FastLED.

- Arduino IDE: 2.3.4
- ESP 8266 board library: 3.1.2
- FastLED: 3.3.3
- EspSoftwareSerial: 8.1.0
- Modbusmaster: 2.0.1
