# BU-Act swtimer Library

## General
* Customer: Melexis
* Product(s): MLX81107/09, MLX81330/32, MLX81340/44
* Description: Libraries

## Getting started
 * The module swtimer allows to create and control software timers
 * The module is using the hardware Core Timer to increment the value of registered software timers

## Hardware resources
 * Core timer
 * Core timer interrupt vector

## Dependencies

## Installation

Add *swtimer* to the BU_LIBS list in the file Makefile.srcs.mk located in the application source folder.

```
BU_LIBS += swtimer
```

## License
Copyright (C) 2019-2020 Melexis N.V.

The Software is being delivered 'AS IS' and Melexis, whether explicitly or implicitly, makes no warranty as to its Use or performance.
The user accepts the Melexis Firmware License Agreement.

Melexis confidential & proprietary
