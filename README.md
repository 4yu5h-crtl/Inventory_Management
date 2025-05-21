# Inventory Management System

This project implements an inventory management system using an ESP8266/NodeMCU microcontroller and an ultrasonic sensor. The system allows you to track the quantity of different items in a box by measuring the distance with the ultrasonic sensor.

## Hardware Requirements

- ESP8266 or NodeMCU board
- HC-SR04 Ultrasonic Sensor
- Jumper wires
- A box to hold the items
- Power supply (USB cable or external power)

## Wiring

Connect the ultrasonic sensor to the ESP8266/NodeMCU as follows:

- Ultrasonic Sensor VCC → ESP8266 3.3V
- Ultrasonic Sensor GND → ESP8266 GND
- Ultrasonic Sensor TRIG → ESP8266 GPIO5
- Ultrasonic Sensor ECHO → ESP8266 GPIO18

## Software Requirements

- Arduino IDE
- ESP8266 board support for Arduino
- Required libraries:
  - ESP8266WiFi
  - ESP8266WebServer
  - ArduinoJson (version 6.x)

## Installation

1. Install the Arduino IDE from [arduino.cc](https://www.arduino.cc/en/software)
2. Add ESP8266 board support to Arduino IDE:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to the "Additional Boards Manager URLs" field
   - Go to Tools > Board > Boards Manager
   - Search for "esp8266" and install the ESP8266 board package
3. Install the required libraries:
   - Go to Tools > Manage Libraries
   - Search for and install:
     - ESP8266WiFi
     - ESP8266WebServer
     - ArduinoJson (version 6.x)

## Configuration

1. Open the `inventory_system.ino` file in Arduino IDE
2. Update the WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
3. Adjust the constants for your specific setup:
   ```cpp
   const int EMPTY_BOX_DISTANCE = 30;  // Distance when box is empty (cm)
   const int ITEM_HEIGHT = 5;  // Approximate height of one item (cm)
   ```
4. Select your board from Tools > Board menu
5. Select the correct port from Tools > Port menu
6. Upload the sketch to your ESP8266/NodeMCU

## Usage

1. After uploading the sketch, open the Serial Monitor (Tools > Serial Monitor) and set the baud rate to 115200
2. The ESP8266 will connect to your WiFi network and display its IP address
3. Open a web browser and navigate to the IP address shown in the Serial Monitor
4. You'll see the inventory management interface with a grid of items
5. To use the system:
   - Click on an item (e.g., "Milk") to select it
   - Place the corresponding items in the box
   - The system will automatically count the items based on the ultrasonic sensor readings
   - The count will be displayed on the web interface
   - Click on another item to switch to counting a different item

## Calibration

The system needs to be calibrated to work correctly:

1. Make sure the box is empty
2. Click the "Calibrate Empty Box" button on the web interface
3. The system will record the current distance as the "empty box" distance
4. Now you can start adding items to the box


## Troubleshooting

- If the sensor readings are inaccurate, try adjusting the `ITEM_HEIGHT` constant
- If the system doesn't connect to WiFi, check your credentials and network
- If the web interface doesn't load, check the IP address in the Serial Monitor
- If the count is incorrect, try recalibrating the empty box

## License

This project is open source and available under the MIT License. 
