# Kerbal Buttons

A simple 4 button and 4 LED control panel for [Kerbal Space Program 1](https://www.kerbalspaceprogram.com/games-kerbal-space-program).

Displays state of and provides input for SAS, RCS, Gear and Lights.

Reads and Writes data to KSP using the [KSP SerialIO plugin]( 
https://github.com/zitron-git/KSPSerialIO) via the serial port.


Built using a tiny [Waveshare RP2040 Zero](https://www.waveshare.com/wiki/RP2040-Zero) and the Arduino environment.

Uses [FastLED](https://fastled.io/docs/index.html) to control the built-in WS2812b on the Zero.


See [HardwareConf.h](HardwareConf.h) for wiring. 


RP2040 Zero has 2 HW Serial Ports as well as the USB SoftSerial

In the final build:
 - USBSerial to talk to KSPIO
 - Serial0 for optional debug output


During development we are resetting the RP2040 a lot, every time we program it. This means the USBSerial port will dissapear/reappear every cycle from KSPSerialIO's point of view. To reconnect you have to revert the flight or sometime restart KSP which takes ages.

So for easier development we can make use of the extra serial ports.
and a couple of [USB to Serial adaptors](https://www.amazon.com/dp/B07K76Q2DX) to provide KSPSerialIO with a stable port and also another to send to a terminal like [Termite](https://www.compuphase.com/software_termite.htm) for debug output. 

The great thing about this is that the serial ports are independent from the RP2040 so they don't close/open every time the RP2040 resets. You can leave KSP and Termite open all the time and RP2040 will pick up where it left off after every reset.

Dev/Debug:
- USBSerial only to communicatre with Arduino IDE
- Serial1 (HW serial 0) debug output (TX only)
- Serial2 (HW Serial 1) communication with KSPSerialIO (RX+TX)


Handy ref: https://arduino-pico.readthedocs.io/en/latest/serial.html


Copyright (C) 2024 Matt Casey : catmacey.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.