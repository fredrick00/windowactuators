#include <WiFi.h>
#include <BluetoothSerial.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <ArduinoJson.h>
#include <HardwareSerial.h>

const char* ssid = "FarFarmFi";
const char* password = "FarmFastWifi";
bool haveWiFi = true; // Boolean to toggle WiFi reconnect attempts

unsigned long lastWiFiAttemptMillis = 0;
const unsigned long wifiAttemptInterval = 60000; // 1 minute

BluetoothSerial SerialBT;

#define LED_BUILTIN 2 // Define LED_BUILTIN if it's not defined

enum ActuatorState { INACTIVE, EXTENDING, RETRACTING };
ActuatorState actuatorStates[4] = { INACTIVE, INACTIVE, INACTIVE, INACTIVE };

const char htmlPage[] = R"rawliteral(
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
    .inactive {
      background-color: white;
    }
    .extending {
      background-color: green;
    }
    .retracting {
      background-color: red;
    }
  </style>
</head>
<body>
  <h1>ESP32 Actuator Control</h1>
  <button onclick="sendCommand('EXTEND', 1)" class="inactive" id="actuator1">Extend Actuator 1</button>
  <button onclick="sendCommand('RETRACT', 1)" class="inactive" id="actuator1">Retract Actuator 1</button><br>
  <button onclick="sendCommand('EXTEND', 2)" class="inactive" id="actuator2">Extend Actuator 2</button>
  <button onclick="sendCommand('RETRACT', 2)" class="inactive" id="actuator2">Retract Actuator 2</button><br>
  <button onclick="sendCommand('EXTEND', 3)" class="inactive" id="actuator3">Extend Actuator 3</button>
  <button onclick="sendCommand('RETRACT', 3)" class="inactive" id="actuator3">Retract Actuator 3</button><br>
  <button onclick="sendCommand('EXTEND', 4)" class="inactive" id="actuator4">Extend Actuator 4</button>
  <button onclick="sendCommand('RETRACT', 4)" class="inactive" id="actuator4">Retract Actuator 4</button><br>
  <button onclick="sendCommand('EXTEND', 0)" class="inactive" id="all">Extend All Actuators</button>
  <button onclick="sendCommand('RETRACT', 0)" class="inactive" id="all">Retract All Actuators</button>
  <script>
    function sendCommand(action, actuator) {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/toggle?action=" + action + "&actuator=" + actuator, true);
      xhr.send();
    }

    function updateStatus() {
      var xhr = new XMLHttpRequest();
      xhr.open("GET", "/status", true);
      xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
          var response = JSON.parse(xhr.responseText);
          for (var i = 0; i < 4; i++) {
            var buttonId = 'actuator' + (i + 1);
            var element = document.getElementById(buttonId);
            if (response.states[i] == 'EXTENDING') {
              element.className = 'extending';
            } else if (response.states[i] == 'RETRACTING') {
              element.className = 'retracting';
            } else {
              element.className = 'inactive';
            }
          }
        }
      };
      xhr.send();
    }

    setInterval(updateStatus, 1000); // Update status every second
  </script>
</body>
</html>
)rawliteral";

/** 
  End of variable declarations and HTML page.
  Beginning of First Class
**/

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
  void handleStatus();
};

WiFiManager wifiManager;
BluetoothManager btManager;
WebServerManager webServerManager;

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
  server.on("/status", [this]() { handleStatus(); });
  server.begin();
  Serial.println(F("HTTP server started"));
}

void WebServerManager::handleRoot() {
  server.send(200, "text/html", htmlPage);
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

    // Update the actuator state
    if (action == "EXTEND") {
      actuatorStates[actuator - 1] = EXTENDING;
    } else if (action == "RETRACT") {
      actuatorStates[actuator - 1] = RETRACTING;
    }
  } else {
    server.send(400, "text/plain", "Invalid command");
  }
}

void WebServerManager::handleStatus() {
  DynamicJsonDocument doc(1024);
  JsonArray states = doc.createNestedArray("states");
  for (int i = 0; i < 4; ++i) {
    String state;
    switch (actuatorStates[i]) {
      case EXTENDING:
        state = "EXTENDING";
        break;
      case RETRACTING:
        state = "RETRACTING";
        break;
      default:
        state = "INACTIVE";
    }
    states.add(state);
  }
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

void setup() {
  Serial.begin(115200);      // Serial communication with the computer
  Serial2.begin(115200, SERIAL_8N1, 27, 28); // Serial communication with Arduino Mega (RX2, TX2)

  pinMode(LED_BUILTIN, OUTPUT);  // Optional: Use built-in LED for testing

  btManager.begin();
  wifiManager.connectToWiFi();
  webServerManager.begin();
}

void loop() {
  btManager.handleBluetooth();
  wifiManager.handleWiFi();
  webServerManager.handleClient();

  if (Serial2.available()) {
    String command = Serial2.readStringUntil('\n');
    Serial.print(F("Received from Mega: "));
    Serial.println(command);

    // Update the actuator state based on the command received from Mega
    if (command.startsWith("EXTEND")) {
      int actuator = command.substring(7).toInt();
      actuatorStates[actuator - 1] = EXTENDING;
    } else if (command.startsWith("RETRACT")) {
      int actuator = command.substring(8).toInt();
      actuatorStates[actuator - 1] = RETRACTING;
    } else if (command == "ALL EXTENDED") {
      for (int i = 0; i < 4; i++) {
        actuatorStates[i] = EXTENDING;
      }
    } else if (command == "ALL RETRACTED") {
      for (int i = 0; i < 4; i++) {
        actuatorStates[i] = RETRACTING;
      }
    }
  }
}
