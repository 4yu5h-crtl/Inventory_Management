#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "your-wifi";
const char* password = "your-password";

// Pin definitions
const int TRIG_PIN = D6;
const int ECHO_PIN = D5;

// Constants
const int MAX_DISTANCE = 200;
const int DETECTION_THRESHOLD = 30;
const int DEBOUNCE_TIME = 1000;
const int SENSOR_READINGS = 5; // Number of readings to average

// Web server
ESP8266WebServer server(80);

// Inventory structure
struct InventoryItem {
  String name;
  int count;
};

const int MAX_ITEMS = 10;
InventoryItem inventory[MAX_ITEMS] = {
  {"Milk", 0}, {"Chocolate", 0}, {"Bread", 0}, {"Eggs", 0}, {"Cereal", 0},
  {"Pen", 0}, {"Pencil", 0}, {"Notebook", 0}, {"Book", 0}, {"Adhesive Tape", 0}
};

int currentItemIndex = 0;
bool objectPreviouslyDetected = false;
unsigned long lastDetectionTime = 0;

void setup() {
  Serial.begin(9600);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.println("Connected, IP address: " + WiFi.localIP().toString());

  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/inventory", HTTP_GET, handleGetInventory);
  server.on("/api/select-item", HTTP_POST, handleSelectItem);
  server.on("/api/reset-count", HTTP_POST, handleResetCount);
  server.on("/api/reduce-count", HTTP_POST, handleReduceCount);
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  server.handleClient();
  
  // Get average distance from multiple readings
  int distance = getAverageDistance();
  bool objectCurrentlyDetected = distance < DETECTION_THRESHOLD;

  // Only increment count if:
  // 1. Object is currently detected
  // 2. Object was not previously detected (to avoid multiple counts)
  // 3. Enough time has passed since last detection (debounce)
  if (objectCurrentlyDetected && !objectPreviouslyDetected && millis() - lastDetectionTime > DEBOUNCE_TIME) {
    if (currentItemIndex >= 0 && currentItemIndex < MAX_ITEMS) {
      inventory[currentItemIndex].count++;
      Serial.println("Detected: " + inventory[currentItemIndex].name + " -> " + String(inventory[currentItemIndex].count));
      lastDetectionTime = millis();
    }
  }

  objectPreviouslyDetected = objectCurrentlyDetected;
  delay(100);
}

// Get average distance from multiple readings to reduce noise
int getAverageDistance() {
  int totalDistance = 0;
  int validReadings = 0;
  
  for (int i = 0; i < SENSOR_READINGS; i++) {
    int distance = readUltrasonicDistance();
    if (distance > 0 && distance < MAX_DISTANCE) {
      totalDistance += distance;
      validReadings++;
    }
    delay(10); // Small delay between readings
  }
  
  // Return average if we have valid readings, otherwise return MAX_DISTANCE
  return (validReadings > 0) ? totalDistance / validReadings : MAX_DISTANCE;
}

int readUltrasonicDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  
  // Increase timeout to 50ms for more reliable readings
  long duration = pulseIn(ECHO_PIN, HIGH, 50000);
  
  // Check if we got a valid reading
  if (duration == 0) {
    return MAX_DISTANCE; // Return max distance if no echo received
  }
  
  int distance = duration * 0.034 / 2;
  return (distance > MAX_DISTANCE) ? MAX_DISTANCE : distance;
}

void handleRoot() {
  server.send(200, "text/html", R"rawliteral(
<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width, initial-scale=1.0'>
<title>Inventory System</title><style>
body{font-family:Arial;background:#f5f5f5;padding:20px}
.container{max-width:800px;margin:auto;background:white;padding:20px;border-radius:8px;box-shadow:0 2px 4px rgba(0,0,0,0.1)}
.instructions{background:#e3f2fd;padding:15px;border-radius:4px;margin-bottom:20px}
.item-grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(150px,1fr));gap:15px;margin-top:20px}
.item-card{background:#f9f9f9;border:1px solid #ddd;border-radius:4px;padding:15px;text-align:center;cursor:pointer;transition:0.3s}
.item-card:hover{transform:translateY(-5px);box-shadow:0 5px 15px rgba(0,0,0,0.1)}
.item-card.selected{background:#e3f2fd;border-color:#2196f3}
.item-count{font-size:24px;font-weight:bold;margin:10px 0;color:#2196f3}
.controls{margin-top:20px;display:flex;justify-content:space-between}
button{background:#2196f3;color:white;border:none;padding:10px 15px;border-radius:4px;cursor:pointer;margin-right:5px}
button:hover{background:#0b7dda}
button.remove{background:#f44336}
button.remove:hover{background:#d32f2f}
.status-bar{margin-top:15px;padding:10px;background:#f0f0f0;border-radius:4px;text-align:center;font-size:14px;color:#666}
</style></head><body><div class='container'><h1>Inventory Management</h1>
<div class='instructions'><h3>Instructions:</h3><ol>
<li>Click an item to select it</li><li>Place an object in front of the sensor to add items</li><li>Click "Remove Item" to reduce count</li><li>Use Reset to clear count</li></ol></div>
<div class='item-grid' id='itemGrid'></div>
<div class='controls'><button id='resetBtn'>Reset Count</button><button id='removeBtn' class='remove'>Remove Item</button><button id='refreshBtn'>Refresh Now</button></div>
<div class='status-bar' id='statusBar'>Last updated: Never</div></div>
<script>
let selectedItemIndex=-1;let inventory=[];
let autoRefreshInterval = 1000; // Refresh every 1 second
let lastUpdateTime = new Date();

function updateStatusBar() {
  const now = new Date();
  const timeDiff = Math.floor((now - lastUpdateTime) / 1000);
  document.getElementById('statusBar').textContent = `Last updated: ${timeDiff} seconds ago`;
}

function fetchInventory(){fetch('/api/inventory').then(r=>r.json()).then(data=>{inventory=data;updateItemGrid();lastUpdateTime=new Date();updateStatusBar();})}
function updateItemGrid(){const g=document.getElementById('itemGrid');g.innerHTML='';
inventory.forEach((item,i)=>{if(item.name){const c=document.createElement('div');
c.className='item-card'+(i===selectedItemIndex?' selected':'');
c.onclick=()=>selectItem(i);
c.innerHTML=`<div>${item.name}</div><div class='item-count'>${item.count}</div>`;g.appendChild(c);}});}
function selectItem(index){selectedItemIndex=index;
fetch('/api/select-item',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({index})}).then(()=>updateItemGrid())}
document.getElementById('resetBtn').addEventListener('click',()=>{if(selectedItemIndex===-1)return;
fetch('/api/reset-count',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({index:selectedItemIndex})}).then(()=>fetchInventory())});
document.getElementById('removeBtn').addEventListener('click',()=>{if(selectedItemIndex===-1)return;
fetch('/api/reduce-count',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({index:selectedItemIndex})}).then(()=>fetchInventory())});
document.getElementById('refreshBtn').addEventListener('click',fetchInventory);

// Set up automatic refresh
fetchInventory(); // Initial load
setInterval(fetchInventory, autoRefreshInterval); // Auto refresh every 1 second
setInterval(updateStatusBar, 1000); // Update status bar every second
</script></body></html>)rawliteral");
}

void handleGetInventory() {
  DynamicJsonDocument doc(1024);
  JsonArray items = doc.to<JsonArray>();
  for (int i = 0; i < MAX_ITEMS; i++) {
    JsonObject item = items.createNestedObject();
    item["name"] = inventory[i].name;
    item["count"] = inventory[i].count;
  }
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void handleSelectItem() {
  DynamicJsonDocument doc(256);
  deserializeJson(doc, server.arg("plain"));
  currentItemIndex = doc["index"];
  server.send(200, "text/plain", "Item selected");
}

void handleResetCount() {
  DynamicJsonDocument doc(256);
  deserializeJson(doc, server.arg("plain"));
  int index = doc["index"];
  if (index >= 0 && index < MAX_ITEMS) {
    inventory[index].count = 0;
  }
  server.send(200, "text/plain", "Count reset");
}

void handleReduceCount() {
  DynamicJsonDocument doc(256);
  deserializeJson(doc, server.arg("plain"));
  int index = doc["index"];
  if (index >= 0 && index < MAX_ITEMS) {
    // Ensure count doesn't go below zero
    if (inventory[index].count > 0) {
      inventory[index].count--;
      Serial.println("Reduced: " + inventory[index].name + " -> " + String(inventory[index].count));
    }
  }
  server.send(200, "text/plain", "Count reduced");
}
