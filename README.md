# DS3231M library
<img src="https://github.com/SV-Zanshin/DS3231M/blob/master/Images/ds3231m-real-time-clock-ic.jpg" width="175" align="right"/> *Arduino* library for accessing the maxim integrated DS3231M real time clock. This clock requires no external components and with an intern temperature-compensated oscillator it is extremely accurate.

## Overview
The [DS3231M datasheet](https://datasheets.maximintegrated.com/en/ds/DS3231M.pdf) and [DS3231MN](http://ww1.microchip.com/downloads/en/DeviceDoc/20005010F.pdf) describes the chip in detail. While there is no breakout board available for this RTC at the present time, it is not difficult to build into a project. The DS3231M is available as a PDIP 8 pin which allows for easy use on a breadboard and the only additional parts necessary are a 32.768kHz crystal with 2 small capacitors (~6pF, depends upon circuit capacitance) and some pull-up resistors for the I2C data lines as well as for the multifunction pin, if that is to be used.

## Features
Apart from performing as a real-time clock, the MCP4790 has 64 Bytes of SRAM storage available and features 2 separate alarms that can be set to be recurring or a single use. The alarm state can be checked using library functions or can pull down the the MFP (multi-function pin) for a hardware interrupt or other action.
The DS3231M allows for software trimming and the library has functions which support fine-tuning the RTC to increase accuracy.

## Description
<img src="https://github.com/SV-Zanshin/DS3231M/blob/master/Images/DS3231M_bb.png" width="175px" align="left" /> The detailed library description along with further details are available at the GitHub [DS3231M Wiki](https://github.com/SV-Zanshin/DS3231M/wiki). This includes wiring information and sample sketches that illustrate the library and it's functionality.

![Zanshin Logo](https://www.sv-zanshin.com/r/images/site/gif/zanshinkanjitiny.gif) <img src="https://www.sv-zanshin.com/r/images/site/gif/zanshintext.gif" width="75"/>
