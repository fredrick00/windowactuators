#ifdef ESP32

#include <WiFi.h>
//#include <BluetoothSerial.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "espconfig.h"; 

bool haveWiFi = true; // Boolean to toggle WiFi reconnect attempts

unsigned long lastWiFiAttemptMillis = 0;
const unsigned long wifiAttemptInterval = 60000; // 1 minute

static unsigned long lastBluetoothCheck = 0;
const unsigned long bluetoothCheckInterval = 1000; // Check for Bluetooth connection every second

//BluetoothSerial SerialBT;

#define LED_BUILTIN 2 // Define LED_BUILTIN if it's not defined

unsigned long lastStatusMillis = 0;
const unsigned long statusInterval = 500; // 500ms

enum ActuatorStatus { INACTIVE, EXTENDING, RETRACTING };
ActuatorStatus actuatorStatus[4] = { INACTIVE, INACTIVE, INACTIVE, INACTIVE };

class WiFiManager {
public:
  void connectToWiFi();
  void handleWiFi();
};
/***
  Begin bluetooth comment out
class BluetoothManager {
public:
  void begin();
  void handleBluetooth();
  bool isConnected();
  bool attemptConnection(unsigned long duration); // Attempt connection for a specified duration

private:
  bool connected = false;
};


void BluetoothManager::begin() {
  SerialBT.begin("Makai_Windows"); // Bluetooth device name
}

void BluetoothManager::handleBluetooth() {
  if (SerialBT.hasClient() && !connected) {
    Serial.println("Bluetooth connected");
    connected = true;
  } else if (!SerialBT.hasClient() && connected) {
    Serial.println("Bluetooth disconnected");
    connected = false;
  }

  if (SerialBT.available()) {
    String command = SerialBT.readStringUntil('\n'); // Read command from Bluetooth
    Serial.print("Received from Bluetooth: ");
    Serial.println(command);
    Serial2.println(command); // Forward command to Arduino Mega

    // Test by toggling the built-in LED
    if (command == "TOGGLE") {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    }
  }
}

bool BluetoothManager::isConnected() {
  return connected;
}

bool BluetoothManager::attemptConnection(unsigned long duration) {
  unsigned long startAttemptTime = millis();
  while (!connected && (millis() - startAttemptTime) < duration) {
    handleBluetooth();
    delay(100);
  }
  return connected;
}


  End Bluetooth comment out
***/

class WebServerManager {
public:
  WebServerManager() : server(80) {}
  void begin();
  void handleClient();

private:
  WebServer server;
  void handleRoot();
  void handleToggle();
  void sendStatus();
  void handleNetworkInfo();
  String generateHTML();
};

WiFiManager wifiManager;
// BluetoothManager btManager;
WebServerManager webServerManager;

void setup() {
  Serial.begin(115200);      // Serial communication with the computer
  Serial2.begin(115200, SERIAL_8N1, 27, 28); // Serial communication with Arduino Mega (RX2, TX2)

  pinMode(LED_BUILTIN, OUTPUT);  // Optional: Use built-in LED for testing

//  btManager.begin();
  wifiManager.connectToWiFi();
  webServerManager.begin();
}

void WiFiManager::connectToWiFi() {
  Serial.println("Attempting to connect to WiFi...");
  WiFi.begin(ssid, password);
  lastWiFiAttemptMillis = millis();

  // Try to connect for 10 seconds
  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Connected to WiFi");
    // Announce service to mDNS
    if (!MDNS.begin("esp32")) { // Set the hostname to "esp32"
      Serial.println("Error setting up MDNS responder!");
    } else {
      Serial.println("mDNS responder started");
      MDNS.addService("http", "tcp", 80);
    }
  } else {
    Serial.println("Failed to connect to WiFi");
  }
}

void WiFiManager::handleWiFi() {
  // Periodically attempt to reconnect to Wi-Fi if not connected and haveWiFi is true
  if (haveWiFi && WiFi.status() != WL_CONNECTED && millis() - lastWiFiAttemptMillis >= wifiAttemptInterval) {
    connectToWiFi();
  }
}

void WebServerManager::begin() {
  server.on("/", [this]() { handleRoot(); });
  server.on("/toggle", [this]() { handleToggle(); });
  server.on("/status", [this]() { sendStatus(); });
  server.on("/network", [this]() { handleNetworkInfo(); });
  server.begin();
  Serial.println("HTTP server started");
}

void WebServerManager::handleClient() {
  server.handleClient(); // Handle web server requests
}

void WebServerManager::handleRoot() {
  server.send(200, "text/html", generateHTML());
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

void WebServerManager::sendStatus() {
  DynamicJsonDocument doc(1024);
  JsonArray states = doc.createNestedArray("states");
  for (int i = 0; i < 4; ++i) {
    String state;
    switch (actuatorStatus[i]) {
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

void WebServerManager::handleNetworkInfo() {
  DynamicJsonDocument doc(1024);
  doc["esp_ip"] = WiFi.localIP().toString();
  String json;
  serializeJson(doc, json);
  server.send(200, "application/json", json);
}

String WebServerManager::generateHTML() {
  String html = R"rawliteral(
    <!DOCTYPE HTML>
    <html>
    <head>
      <title>ESP32 Actuator Control</title>
      <style>
        .button {
          padding: 15px 25px;
          font-size: 24px;
          margin: 5px;
          border: none;
          color: white;
        }
        .extend-inactive { background-color: blue; }
        .extend-extending { background-color: green; }
        .extend-retracting { background-color: red; }
        .retract-inactive { background-color: blue; }
        .retract-extending { background-color: green; }
        .retract-retracting { background-color: red; }
      </style>
    </head>
    <body>
      <h1>ESP32 Actuator Control</h1>
  )rawliteral";

  for (int i = 0; i < 4; i++) {
    String extendClass = "extend-inactive";
    String retractClass = "retract-inactive";

    if (actuatorStatus[i] == EXTENDING) {
      extendClass = "extend-extending";
    } else if (actuatorStatus[i] == RETRACTING) {
      retractClass = "retract-retracting";
    } else if (actuatorStatus[i] == INACTIVE) {
      if (extendClass == "extend-extending") {
        extendClass = "extend-inactive";
      } else if (retractClass == "retract-retracting") {
        retractClass = "retract-inactive";
      }
    }

    html += "<button type='button' class='button " + extendClass + "' onclick=\"sendCommand('EXTEND', " + String(i + 1) + ")\" id='extend" + String(i + 1) + "'>Extend Actuator " + String(i + 1) + "</button>";
    html += "<button type='button' class='button " + retractClass + "' onclick=\"sendCommand('RETRACT', " + String(i + 1) + ")\" id='retract" + String(i + 1) + "'>Retract Actuator " + String(i + 1) + "</button><br>";
  }

  html += R"rawliteral(
      <button type='button' class="button extend-inactive" onclick="sendCommand('EXTEND', 0)">Extend All Actuators</button>
      <button type='button' class="button retract-inactive" onclick="sendCommand('RETRACT', 0)">Retract All Actuators</button>
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
                var extendElement = document.getElementById('extend' + (i + 1));
                var retractElement = document.getElementById('retract' + (i + 1));
                if (response.states[i] == 'EXTENDING') {
                  extendElement.className = 'button extend-extending';
                  retractElement.className = 'button retract-inactive';
                } else if (response.states[i] == 'RETRACTING') {
                  retractElement.className = 'button retract-retracting';
                  extendElement.className = 'button extend-inactive';
                } else {
                  extendElement.className = 'button extend-inactive';
                  retractElement.className = 'button retract-inactive';
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

  return html;
}

void loop() {
 // btManager.handleBluetooth();
  wifiManager.handleWiFi();
  webServerManager.handleClient();
/**
  if (millis() - lastBluetoothCheck >= bluetoothCheckInterval) {
    lastBluetoothCheck = millis();
    btManager.handleBluetooth();
  }
  **/

  if (Serial2.available()) {
    String status = Serial2.readStringUntil('\n');
    Serial.print("Status from Mega: ");
    Serial.println(status);

    // Process the status here
    for (int i = 0; i < 4; i++) {
      actuatorStatus[i] = INACTIVE; // Reset status
    }

    int index = status.indexOf("STATUS ");
    if (index >= 0) {
      status = status.substring(index + 7);
      while (status.length() > 0) {
        int spaceIndex = status.indexOf(' ');
        if (spaceIndex < 0) spaceIndex = status.length();
        String token = status.substring(0, spaceIndex);
        status = status.substring(spaceIndex + 1);

        if (token.length() > 1) {
          int actuator = token[0] - '1';
          char state = token[1];
          if (state == 'E') {
            actuatorStatus[actuator] = EXTENDING;
          } else if (state == 'R') {
            actuatorStatus[actuator] = RETRACTING;
          }
        }
      }
    }
  }
}

#endif
