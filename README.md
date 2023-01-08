# xbee-esp32-interface
This is a C++ interface that connects an ESP32 Microcontroller to a XBee module operating in API mode. This interface provides functions that allow an ESP32 to send and receive XBee Frames using a physical serial connection (UART, I2C, SPI) with a XBee module.
## Installation
- Add ``xbee_api.cpp`` to the ``src`` directory of the ESP32 project and add ``#include "xbee_api.hpp"`` to the ``main`` source file  
- Add ``xbee_api.hpp`` and ``json.hpp`` to the ``include`` directory of the ESP32 project 

## Licensing Information
This file uses json.hpp by Niels Lohmann which is licensed under the [MIT License](https://opensource.org/licenses/MIT). Copyright &copy; 2013-2022 [Niels Lohmann](https://nlohmann.me).
