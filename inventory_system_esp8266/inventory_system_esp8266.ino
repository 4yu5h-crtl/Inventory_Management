#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "your-wifi-ssid";
const char* password = "your-wifi-password";

// Pin definitions
const int TRIG_PIN = 5;  // Ultrasonic sensor trigger pin (GPIO5)
const int ECHO_PIN = 18;  // Ultrasonic sensor echo pin (GPIO18)

// Constants for ultrasonic sensor
const int MAX_DISTANCE = 200;  // Maximum distance to measure (cm)
const int DETECTION_THRESHOLD = 30;  // Distance threshold for object detection (cm)
const int DEBOUNCE_TIME = 1000;  // Debounce time in milliseconds

// Web server
ESP8266WebServer server(80);

// Inventory data
struct InventoryItem {
  String name;
  int count;
};

const int MAX_ITEMS = 10;
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

int currentItemIndex = 0;  // Index of the currently selected item
bool objectDetected = false;  // Flag to track if an object is currently detected
unsigned long lastDetectionTime = 0;  // Time of the last detection

void setup() {
  Serial.begin(115200);
  
  // Initialize ultrasonic sensor pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  
  // Connect to WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Connected to WiFi. IP address: ");
  Serial.println(WiFi.localIP());
  
  // Set up web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/inventory", HTTP_GET, handleGetInventory);
  server.on("/api/select-item", HTTP_POST, handleSelectItem);
  server.on("/api/reset-count", HTTP_POST, handleResetCount);
  
  // Start web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Read ultrasonic sensor
  int distance = readUltrasonicDistance();
  
  // Check if an object is detected (distance less than threshold)
  bool currentObjectDetected = (distance < DETECTION_THRESHOLD);
  
  // If object is detected and wasn't detected before, and enough time has passed since last detection
  if (currentObjectDetected && !objectDetected && (millis() - lastDetectionTime > DEBOUNCE_TIME)) {
    // Increment count for the selected item
    if (currentItemIndex < MAX_ITEMS) {
      inventory[currentItemIndex].count++;
      Serial.print("Object detected! Updated count for ");
      Serial.print(inventory[currentItemIndex].name);
      Serial.print(": ");
      Serial.println(inventory[currentItemIndex].count);
      
      // Update the last detection time
      lastDetectionTime = millis();
    }
  }
  
  // Update the object detection state
  objectDetected = currentObjectDetected;
  
  delay(100);  // Small delay to prevent too frequent readings
}

// Function to read distance from ultrasonic sensor
int readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  long duration = pulseIn(ECHO_PIN, HIGH);
  int distance = duration * 0.034 / 2;  // Calculate distance in cm
  
  // Limit distance to maximum
  if (distance > MAX_DISTANCE) {
    distance = MAX_DISTANCE;
  }
  
  return distance;
}

// Web server handlers
void handleRoot() {
  String html = "<!DOCTYPE html>";
  html += "<html>";
  html += "<head>";
  html += "<meta charset='UTF-8'>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<title>Inventory Management System</title>";
  html += "<style>";
  html += "body { font-family: Arial, sans-serif; margin: 0; padding: 20px; background-color: #f5f5f5; }";
  html += "h1 { color: #333; text-align: center; }";
  html += ".container { max-width: 800px; margin: 0 auto; background-color: white; padding: 20px; border-radius: 8px; box-shadow: 0 2px 4px rgba(0,0,0,0.1); }";
  html += ".item-grid { display: grid; grid-template-columns: repeat(auto-fill, minmax(150px, 1fr)); gap: 15px; margin-top: 20px; }";
  html += ".item-card { background-color: #f9f9f9; border: 1px solid #ddd; border-radius: 4px; padding: 15px; text-align: center; cursor: pointer; transition: all 0.3s; }";
  html += ".item-card:hover { transform: translateY(-5px); box-shadow: 0 5px 15px rgba(0,0,0,0.1); }";
  html += ".item-card.selected { background-color: #e3f2fd; border-color: #2196f3; }";
  html += ".item-count { font-size: 24px; font-weight: bold; margin: 10px 0; color: #2196f3; }";
  html += ".controls { margin-top: 20px; display: flex; justify-content: space-between; }";
  html += "button { background-color: #2196f3; color: white; border: none; padding: 10px 15px; border-radius: 4px; cursor: pointer; }";
  html += "button:hover { background-color: #0b7dda; }";
  html += ".instructions { background-color: #fffde7; padding: 15px; border-radius: 4px; margin-top: 20px; border-left: 4px solid #ffd600; }";
  html += "</style>";
  html += "</head>";
  html += "<body>";
  html += "<div class='container'>";
  html += "<h1>Inventory Management System</h1>";
  html += "<div class='instructions'>";
  html += "<p><strong>How to use:</strong></p>";
  html += "<ol>";
  html += "<li>Click on an item to select it</li>";
  html += "<li>Place an object in front of the ultrasonic sensor</li>";
  html += "<li>The count will increase by 1 each time an object is detected</li>";
  html += "<li>Click 'Reset Count' to reset the count for the selected item</li>";
  html += "</ol>";
  html += "</div>";
  html += "<div class='item-grid' id='itemGrid'>";
  // Items will be populated by JavaScript
  html += "</div>";
  html += "<div class='controls'>";
  html += "<button id='resetBtn'>Reset Count</button>";
  html += "<button id='refreshBtn'>Refresh</button>";
  html += "</div>";
  html += "</div>";
  
  html += "<script>";
  html += "let selectedItemIndex = -1;";
  html += "let inventory = [];";
  
  // Function to fetch inventory data
  html += "function fetchInventory() {";
  html += "  fetch('/api/inventory')";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      inventory = data;";
  html += "      updateItemGrid();";
  html += "    })";
  html += "    .catch(error => console.error('Error fetching inventory:', error));";
  html += "}";
  
  // Function to update the item grid
  html += "function updateItemGrid() {";
  html += "  const itemGrid = document.getElementById('itemGrid');";
  html += "  itemGrid.innerHTML = '';";
  html += "  inventory.forEach((item, index) => {";
  html += "    if (item.name) {";  // Only show items with names
  html += "      const itemCard = document.createElement('div');";
  html += "      itemCard.className = 'item-card' + (index === selectedItemIndex ? ' selected' : '');";
  html += "      itemCard.innerHTML = `";
  html += "        <div>${item.name}</div>";
  html += "        <div class='item-count'>${item.count}</div>";
  html += "      `;";
  html += "      itemCard.onclick = () => selectItem(index);";
  html += "      itemGrid.appendChild(itemCard);";
  html += "    }";
  html += "  });";
  html += "}";
  
  // Function to select an item
  html += "function selectItem(index) {";
  html += "  selectedItemIndex = index;";
  html += "  fetch('/api/select-item', {";
  html += "    method: 'POST',";
  html += "    headers: { 'Content-Type': 'application/json' },";
  html += "    body: JSON.stringify({ index: index })";
  html += "  })";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      if (data.success) {";
  html += "        updateItemGrid();";
  html += "      }";
  html += "    })";
  html += "    .catch(error => console.error('Error selecting item:', error));";
  html += "}";
  
  // Function to reset count
  html += "function resetCount() {";
  html += "  if (selectedItemIndex >= 0) {";
  html += "    fetch('/api/reset-count', {";
  html += "      method: 'POST',";
  html += "      headers: { 'Content-Type': 'application/json' },";
  html += "      body: JSON.stringify({ index: selectedItemIndex })";
  html += "    })";
  html += "      .then(response => response.json())";
  html += "      .then(data => {";
  html += "        if (data.success) {";
  html += "          fetchInventory();";
  html += "        }";
  html += "      })";
  html += "      .catch(error => console.error('Error resetting count:', error));";
  html += "  } else {";
  html += "    alert('Please select an item first');";
  html += "  }";
  html += "}";
  
  // Initialize the page
  html += "document.addEventListener('DOMContentLoaded', () => {";
  html += "  fetchInventory();";
  html += "  document.getElementById('resetBtn').onclick = resetCount;";
  html += "  document.getElementById('refreshBtn').onclick = fetchInventory;";
  html += "  // Auto-refresh every 2 seconds";
  html += "  setInterval(fetchInventory, 2000);";
  html += "});";
  
  html += "</script>";
  html += "</body>";
  html += "</html>";
  
  server.send(200, "text/html", html);
}

void handleGetInventory() {
  StaticJsonDocument<1024> doc;
  JsonArray array = doc.to<JsonArray>();
  
  for (int i = 0; i < MAX_ITEMS; i++) {
    if (inventory[i].name.length() > 0) {
      JsonObject item = array.createNestedObject();
      item["name"] = inventory[i].name;
      item["count"] = inventory[i].count;
    }
  }
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSelectItem() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      int index = doc["index"];
      if (index >= 0 && index < MAX_ITEMS) {
        currentItemIndex = index;
        
        StaticJsonDocument<200> response;
        response["success"] = true;
        response["message"] = "Selected item: " + inventory[currentItemIndex].name;
        
        String responseStr;
        serializeJson(response, responseStr);
        server.send(200, "application/json", responseStr);
        return;
      }
    }
  }
  
  server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid request\"}");
}

void handleResetCount() {
  if (server.hasArg("plain")) {
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, server.arg("plain"));
    
    if (!error) {
      int index = doc["index"];
      if (index >= 0 && index < MAX_ITEMS) {
        inventory[index].count = 0;
        
        StaticJsonDocument<200> response;
        response["success"] = true;
        response["message"] = "Reset count for " + inventory[index].name;
        
        String responseStr;
        serializeJson(response, responseStr);
        server.send(200, "application/json", responseStr);
        return;
      }
    }
  }
  
  server.send(400, "application/json", "{\"success\":false,\"message\":\"Invalid request\"}");
} 