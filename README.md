# 📦 Inventory Management System

<div align="center">
  <img src="https://img.shields.io/badge/ESP8266-NodeMCU-blue" alt="ESP8266/NodeMCU">
  <img src="https://img.shields.io/badge/Sensor-HC--SR04-green" alt="HC-SR04 Sensor">
  <img src="https://img.shields.io/badge/Arduino-Compatible-orange" alt="Arduino Compatible">
</div>

## 🚀 Overview

This project implements an inventory management system using an ESP8266/NodeMCU microcontroller and an ultrasonic sensor. The system allows you to track the quantity of different items by detecting objects in front of the ultrasonic sensor.

## 🔧 Hardware Requirements

- **ESP8266 or NodeMCU board**
- **HC-SR04 Ultrasonic Sensor**
- **Jumper wires**
- **Power supply** (USB cable or external power)

## 🔌 Wiring Diagram

Connect the ultrasonic sensor to the ESP8266/NodeMCU as follows:

| Ultrasonic Sensor | ESP8266/NodeMCU |
|-------------------|-----------------|
| VCC               | 3.3V            |
| GND               | GND             |
| TRIG              | D5              |
| ECHO              | D6              |

## 💻 Software Requirements

- **Arduino IDE**
- **ESP8266 board support** for Arduino
- **Required libraries**:
  - ESP8266WiFi
  - ESP8266WebServer
  - ArduinoJson (version 6.x)

## 📥 Installation

1. **Install the Arduino IDE** from [arduino.cc](https://www.arduino.cc/en/software)
2. **Add ESP8266 board support** to Arduino IDE:
   - Open Arduino IDE
   - Go to File > Preferences
   - Add `http://arduino.esp8266.com/stable/package_esp8266com_index.json` to the "Additional Boards Manager URLs" field
   - Go to Tools > Board > Boards Manager
   - Search for "esp8266" and install the ESP8266 board package
3. **Install the required libraries**:
   - Go to Tools > Manage Libraries
   - Search for and install:
     - ESP8266WiFi
     - ESP8266WebServer
     - ArduinoJson (version 6.x)

## ⚙️ Configuration

1. Open the `inventory_system.ino` file in Arduino IDE
2. Update the WiFi credentials:
   ```cpp
   const char* ssid = "YOUR_WIFI_SSID";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
3. Adjust the constants for your specific setup:
   ```cpp
   const int DETECTION_THRESHOLD = 30;  // Distance threshold for object detection (cm)
   const int DEBOUNCE_TIME = 1000;  // Debounce time in milliseconds
   ```
4. Select your board from Tools > Board menu
5. Select the correct port from Tools > Port menu
6. Upload the sketch to your ESP8266/NodeMCU

## 🚀 Usage

1. After uploading the sketch, open the Serial Monitor (Tools > Serial Monitor) and set the baud rate to 115200
2. The ESP8266 will connect to your WiFi network and display its IP address
3. Open a web browser and navigate to the IP address shown in the Serial Monitor
4. You'll see the inventory management interface with a grid of items
5. To use the system:
   - Click on an item (e.g., "Milk") to select it
   - Place an object in front of the ultrasonic sensor
   - The count will increase by 1 each time an object is detected
   - Click on another item to switch to counting a different item
   - Click "Reset Count" to reset the count for the selected item

## 🔍 How It Works

The system uses a simple detection mechanism:
1. When you select an item on the web interface, the system starts monitoring for objects
2. When an object is detected in front of the ultrasonic sensor (distance less than the threshold), the count for the selected item increases by 1
3. A debounce mechanism prevents multiple counts for a single object
4. The web interface automatically refreshes every 2 seconds to show the updated counts

## 🛠️ Customization

You can customize the inventory items by modifying the `inventory` array in the code:

```cpp
InventoryItem inventory[MAX_ITEMS] = {
  {"Milk", 0},
  {"Chocolate", 0},
  {"Bread", 0},
  {"Eggs", 0},
  {"Cereal", 0},
  {"Pen", 0}, 
  {"Pencil", 0},
  {"Notebook", 0},
  {"Book", 0},
  {"Adhesive Tape", 0}
};
```

## 🔧 Troubleshooting

| Issue | Solution |
|-------|----------|
| Sensor too sensitive/not sensitive enough | Adjust the `DETECTION_THRESHOLD` constant |
| Multiple counts for a single object | Increase the `DEBOUNCE_TIME` constant |
| System doesn't connect to WiFi | Check your credentials and network |
| Web interface doesn't load | Check the IP address in the Serial Monitor |

---
