# knockSensor
This project demonstrates how to use an ESP32 to read data from a DHT11 temperature and humidity sensor, and an SW-18010P knocking sensor. The data is sent to a web server over WiFi and to a BLE characteristic.

# Table of Contents
* Hardware
* Libraries
* Setup
* Usage
* Code Overview

# Hardware
* ESP32 Dev Board
* DHT11 Temperature & Humidity Sensor
* SW-18010P Knocking Sensor

# Libraries
This project uses the following libraries:
* [DHT Sensor Library](https://reference.arduino.cc/reference/en/libraries/dht-sensor-library/)
* [ArduinoBLE Library](https://docs.arduino.cc/libraries/arduinoble/)
* [WiFi](https://docs.arduino.cc/libraries/wifi/)
* [ESPAsyncWebServer](https://docs.arduino.cc/libraries/espasyncwebserver/)

# Setup
1. Connect the DHT11 sensor:
     * Data pin to GPIO 13
     * Power and Ground pins to 3.3V and GND
2. Connect the SW-18010P knocking sensor:
     * Data pin to GPIO 16
     * Power and Ground pins to 3.3V and GND
3. Install the required libraries in your Arduino IDE:
     * DHT Sensor Library
     * ArduinoBLE Library
     * WiFi
     * ESPAsyncWebServer
4. Configure WiFi credentials in the code:
     * `const char* ssid = "your_SSID";`
     * `const char* password = "your_PASSWORD";`

# Usage
* Upload the code to your ESP32 board.
* Open the Serial Monitor to check the connection status and sensor readings.
* Connect to the ESP32 over WiFi and navigate to the IP address shown in the Serial Monitor to see the web page displaying sensor data.
* Use a BLE client to connect to the ESP32 and read the sensor data via the BLE characteristic.

# Code Overview
## Libraries and Definitions
`#include "DHT.h"
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include <WebServer.h>`
* DHT.h: For reading temperature and humidity data from the DHT11 sensor.
* BLEDevice.h, BLEServer.h, BLEUtils.h: For BLE communication.
* WiFi.h: To connect to WiFi.
* WebServer.h: To create a web server for displaying sensor data.
# Global Variables
## WiFi credentials:
`const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";`
## Sensor values:
`String temperature = "N/A";
String humidity = "N/A";
String knocking = "Nikdo neklepe";`
# Functions
* getTimeAgo(): Returns the time elapsed since the last data update.
* handleRoot(): Serves the HTML page displaying sensor data.
* setup(): Initializes the sensors, WiFi, web server, and BLE.
* loop(): Reads sensor data, updates the BLE characteristic, and handles web server requests.

# Additional Notes
* The web server refreshes every second to display updated sensor data.
* BLE characteristic updates with the latest sensor values every second.
