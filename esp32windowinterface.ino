#include <WiFi.h>
#include <BluetoothSerial.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HardwareSerial.h>

const char* ssid = "FarFarmFi";
const char* password = "FarmFastWifi";
bool haveWiFi = true; // Boolean to toggle WiFi reconnect attempts

unsigned long lastWiFiAttemptMillis = 0;
const unsigned long wifiAttemptInterval = 60000; // 1 minute

BluetoothSerial SerialBT;

#define LED_BUILTIN 2 // Define LED_BUILTIN if it's not defined

class WiFiManager {
public:
  void connectToWiFi();
  void handleWiFi();
};

class BluetoothManager {
public:
  void begin();
  void handleBluetooth();
};

class WebServerManager {
public:
  WebServerManager() : server(80) {}
  void begin();
  void handleClient();

private:
  WebServer server;
  void handleRoot();
  void handleToggle();
};

WiFiManager wifiManager;
BluetoothManager btManager;
WebServerManager webServerManager;

void setup() {
  Serial.begin(115200);      // Serial communication with the computer
  Serial2.begin(115200, SERIAL_8N1, 16, 17); // Serial communication with Arduino Mega (RX2, TX2)

  pinMode(LED_BUILTIN, OUTPUT);  // Optional: Use built-in LED for testing

  btManager.begin();
  wifiManager.connectToWiFi();
  webServerManager.begin();
}

void loop() {
  btManager.handleBluetooth();
  webServerManager.handleClient();
  wifiManager.handleWiFi();
}

void WiFiManager::connectToWiFi() {
  Serial.println(F("Attempting to connect to WiFi..."));
  WiFi.begin(ssid, password);
  lastWiFiAttemptMillis = millis();

  // Try to connect for 10 seconds
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(F("."));
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F("Connected to WiFi"));
    // Announce service to mDNS
    if (!MDNS.begin("esp32")) { // Set the hostname to "esp32"
      Serial.println(F("Error setting up MDNS responder!"));
    } else {
      Serial.println(F("mDNS responder started"));
      MDNS.addService("http", "tcp", 80);
    }
  } else {
    Serial.println(F("Failed to connect to WiFi"));
  }
}

void WiFiManager::handleWiFi() {
  // Periodically attempt to reconnect to Wi-Fi if not connected and haveWiFi is true
  if (haveWiFi && WiFi.status() != WL_CONNECTED && millis() - lastWiFiAttemptMillis >= wifiAttemptInterval) {
    connectToWiFi();
  }
}

void BluetoothManager::begin() {
  SerialBT.begin("ESP32_BT"); // Bluetooth device name
}

void BluetoothManager::handleBluetooth() {
  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n'); // Read command from Bluetooth
    Serial.print(F("Received from Bluetooth: "));
    Serial.println(command);
    Serial2.println(command); // Forward command to Arduino Mega

    // Test by toggling the built-in LED
    if (command == "TOGGLE") {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
}

void WebServerManager::begin() {
  server.on("/", [this]() { handleRoot(); });
  server.on("/toggle", [this]() { handleToggle(); });
  server.begin();
  Serial.println(F("HTTP server started"));
}

void WebServerManager::handleClient() {
  server.handleClient(); // Handle web server requests
}

const char htmlPage[]  = R"rawliteral(
<!DOCTYPE HTML>
<html>
<head>
  <title>ESP32 Actuator Control</title>
  <style>
    button {
      padding: 15px 25px;
      font-size: 24px;
      margin: 5px;
    }
  </style>
</head>
<body>
  <h1>ESP32 Actuator Control</h1>
  <button onclick="sendCommand('EXTEND', 1)">Extend Actuator 1</button>
  <button onclick="sendCommand('RETRACT', 1)">Retract Actuator 1</button><br>
  <button onclick="sendCommand('EXTEND', 2)">Extend Actuator 2</button>
  <button onclick="sendCommand('RETRACT', 2)">Retract Actuator 2</button><br>
  <button onclick="sendCommand('EXTEND', 3)">Extend Actuator 3</button>
  <button onclick="sendCommand('RETRACT', 3)">Retract Actuator 3</button><br>
  <button onclick="sendCommand('EXTEND', 4)">Extend Actuator 4</button>
  <button onclick="sendCommand('RETRACT', 4)">Retract Actuator 4</button><br>
  <button onclick="sendCommand('EXTEND', 0)">Extend All Actuators</button>
  <button onclick="sendCommand('RETRACT', 0)">Retract All Actuators</button>
  <script>
    function sendCommand(action, actuator) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/toggle?action=" + action + "&actuator=" + actuator, true);
      xhr.send();
    }
  </script>
</body>
</html>
)rawliteral";

void WebServerManager::handleRoot() {
  server.send(200, "text/html", FPSTR(htmlPage));
}

void WebServerManager::handleToggle() {
  if (server.hasArg("action") && server.hasArg("actuator")) {
    String action = server.arg("action");
    int actuator = server.arg("actuator").toInt();

    String command;
    if (actuator == 0) { // All actuators
      command = action + " ALL";
    } else { // Individual actuator
      command = action + " " + String(actuator);
    }

    Serial2.println(command);
    server.send(200, "text/plain", "Command sent: " + command);
  } else {
    server.send(400, "text/plain", "Invalid command");
  }
}
