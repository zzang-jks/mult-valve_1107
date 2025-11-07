## Standard LIN/J2602 API
--------------

### Getting started

The LIN API is a network software layer that hides the details of a LIN network configuration
(e.g. how signals are mapped into certain frames) for a user making an application program for an arbitrary ECU.
Instead the user will be provided an API, which is focused on the signals transported on the LIN network.

* Checkout submodules
```sh
$ git submodule update --init --recursive
```

### Implemented functionality overview
Please find detailed description for LIN API functionalities by the following link:
[The LIN API functionalities overview](https://docs.google.com/a/melexis.com/spreadsheets/d/1C9iFyQX_fDlKQvetfhOj1iFhL6lyaSNs7AmgtsSyedg/edit?usp=sharing)

### Supported versions

For Slave API:
- LIN 1.3
- LIN 2.0
- LIN 2.1
- LIN 2.2
- SAE J2602:2012
- ISO 17987:2016

For Master API:
- LIN 1.3
- LIN 2.0
- LIN 2.1
- LIN 2.2
- SAE J2602:2012
- ISO 17987:2016

### Dependencies
LDF-Nodegen >= v1.7.4

### License
Copyright (C) 2016-2019 Melexis N.V.

The Software is being delivered 'AS IS' and Melexis, whether explicitly or implicitly, makes no warranty as to its Use or performance.
The user accepts the Melexis Firmware License Agreement.

Melexis confidential & proprietary