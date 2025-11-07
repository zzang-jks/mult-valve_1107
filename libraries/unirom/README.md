# BU-Act unirom library

## General
* Customer: Melexis
* Product: MLX81330/32, MLX81340/44/46, MLX81160
* Description: Libraries

## Getting started
* A library to access application data from EEPROM (Camcu)
* User configuration is protected by Checksum
* In case of invalid user configuration, reload default data can be done using unirom_ResetUserConfig
* EEPROM is written only in case the data have changed 

## Installation

Add *unirom* to the BU_LIBS list in the file Makefile.srcs.mk located in the application source folder.

```
BU_LIBS += unirom
```

Add *unirom_config.h* file to the code src directory in order to define the user configuration structure.


## License
Copyright (C) 2017-2021 Melexis N.V.

The Software is being delivered 'AS IS' and Melexis, whether explicitly or implicitly, makes no warranty as to its Use or performance.
The user accepts the Melexis Firmware License Agreement.

Melexis confidential & proprietary
