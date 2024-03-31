# Kerbal Buttons

A simple 4 button and 4 LED control panel for [Kerbal Space Program 1](https://www.kerbalspaceprogram.com/games-kerbal-space-program).

Displays state of and provides input for SAS, RCS, Gear and Lights.

Reads and Writes data to KSP using the [KSP SerialIO plugin]( 
https://github.com/zitron-git/KSPSerialIO) via the serial port.


Built using a tiny [Waveshare RP2040 Zero](https://www.waveshare.com/wiki/RP2040-Zero) and the Arduino environment.

Uses [FastLED](https://fastled.io/docs/index.html) to control the built-in WS2812b on the Zero.


See [HardwareConf.h](HardwareConf.h) for wiring. 



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