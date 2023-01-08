# ESP32 XBee Interface
This is a guide on how to configure a computer to interact with ESP32 Microcontrollers and XBee communication modules for application and hardware development. This is the process relevant to my senior design project, which is based on the ESP32 and XBee module. 

## Required Software
- [Visual Studio Code](https://code.visualstudio.com/#alt-downloads)
	- [PlatformIO extension](https://platformio.org/platformio-ide)
- [XCTU and XCTU Drivers](https://hub.digi.com/support/products/xctu/)
- [Silicon Labs USB to UART Bridge Driver](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers?tab=downloads)
## Initial Setup
### Connecting to the ESP32

PlatformIO is a development framework for IoT and Embedded Devices that supports the ESP32. Using PlatformIO, the ESP32 can be programmed in Arduino or ESP's own C based programming language: ESP-IDF.   
  
Steps:

	 1. Install Visual Studio Code
	 2. Install the PlatformIO extension
	 3. Install the Silicon Labs USB to UART Bridge Driver
	 4. Connect the ESP32 Developer Module to the computer using a USB cable
	     - The connection can be verified by checking device manager > ports and looking for "Silicon Labs CP210x USb to UART Bridge (COMX)", where COMX is the port in use by the ESP32

### Connecting to the XBee Module

XCTU is an application provided by DigiKey to test and configure XBee modules. 
Steps:

    1. Install the XCTU Driver
    2. Install XCTU
    3. Connect the XBee module to the Computer via USB cable
    4. Search for XBee modules in XCTU

## Creating a PlatformIO Project
To create a new project in PlatformIO, open Visual Code and click the PlatformIO icon. Select "Projects" then "Create New Project". Enter a project name, select the desired board, and the framework to use. This should look like the following: 

![PlatformIO New Project](https://i.imgur.com/1o5Qpb7.png)  

Click finish and let the project be created. Once complete, a new window will open showing the project directory and the automatically generated ``platformio.ini`` configuration file:

![Window upon creating a project](https://i.imgur.com/vD5L4YW.png)

Application code can be placed under the ``src`` folder, with header files included under the ``include`` folder. Once ready to build a project and flash it to the ESP32, the PlatformIO Project Tasks are used. Clicking ``Build`` builds the current project. Clicking ``Upload`` flashes it to the ESP32. The ``Upload and Monitor`` task can be used to flash the ESP32 and monitor its serial output for debugging. 

![PlatformIO Project Tasks](https://i.imgur.com/CzmbkMF.png)

