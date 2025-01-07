# ATTiny85-Power-Controller

![Power Controller](PCB/3D.PNG)

## Hardware

Currently most of the board can be assembled by JLCPCB, only excluding the DCDC boost converter. Files for PCB and assembly are in the PCB folder.
You only need one filter capacitor, so choose between film or electrolytic. headers are optional.

You can choose between 3.0v, 3.3v or 5v DCDC converters, though it is recommended to use 3.3v so that you waste less power, can plug it into a batter port of a device and can set the cutoff voltage up to 3.3v.
Eg,
[**0.9-5V Boost**](https://vi.aliexpress.com/item/1005003932299815.html)
![DCDC Boost](PCB/Boost.PNG)

## Firmware

You will need to use a Programmer like USBASP to flash the ATTiny85 chip.
Firmware can be built in Platformio using the platformio.ini and main.cpp files.

![Top](PCB/PCB_top.PNG)
![Bottom](PCB/PCB_bottom.PNG)
![Schematic](PCB/Schematic.PNG)
