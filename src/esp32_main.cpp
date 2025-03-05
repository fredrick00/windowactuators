#ifdef ESP32
#define RX_PIN 17
#define TX_PIN 16

#include <WiFi.h>
//#include <BluetoothSerial.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <HardwareSerial.h>
#include <ArduinoJson.h>
#include "espconfig.h"
#include <ArduinoOTA.h>

const String ESP32ScriptVersion = "esp32WindowInterface v1.12";
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

//------------------------------------------------------------------------------
// OTAUpdater Class
//------------------------------------------------------------------------------
class OTAUpdater {
public:
    OTAUpdater() {}

    void beginOTA() {
        // Optionally, set a port and hostname
        ArduinoOTA.setPort(3232);
        ArduinoOTA.setHostname("esp32");

        // Configure OTA event handlers
        ArduinoOTA.onStart([]() {
            String type;
            if (ArduinoOTA.getCommand() == U_FLASH) {
                type = "sketch";
            } else { // U_FS
                type = "filesystem";
            }
            Serial.println("Start updating " + type);
        });
        ArduinoOTA.onEnd([]() {
            Serial.println("\nEnd");
        });
        ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
            Serial.printf("OTA Progress: %u%%\r", progress / (total / 100));
        });
        ArduinoOTA.onError([](ota_error_t error) {
            Serial.printf("OTA Error[%u]: ", error);
            if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
            else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
            else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
            else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
            else if (error == OTA_END_ERROR) Serial.println("End Failed");
        });

        ArduinoOTA.begin();
        Serial.println("OTA updater ready");
    }

    void handleOTA() {
        ArduinoOTA.handle();
    }
};

WiFiManager wifiManager;
// BluetoothManager btManager;
WebServerManager webServerManager;
// Instantiate OTAUpdater globally (alongside WiFiManager and WebServerManager)
OTAUpdater otaUpdater;


void WiFiManager::connectToWiFi() {
  Serial.println("Attempting to connect to WiFi...");
  Serial.print ("SSID: ");
  Serial.println (ssid);
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
        // Print the IP address
    Serial.print("ESP32 IP Address: ");
    Serial.println(WiFi.localIP());
    // Announce service to mDNS
    if (!MDNS.begin("esp32")) { // Set the hostname to "esp32"
      Serial.println("Error setting up MDNS responder!");
    } else {
      Serial.println("mDNS responder started.  Hostname: http://esp32.local");
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
  Serial.print("HTTP server started at: http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
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
    Serial.print ("Executing command: ");
    Serial.println (command);
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
  <title>ESP32 Actuator Control - No Scripts</title>
  <style>
    .button {
      padding: 15px 25px;
      font-size: 24px;
      margin: 5px;
      border: none;
      color: white;
      background-color: blue;
      cursor: pointer;
      display: inline-block; /* Ensure buttons are inline-block */
    }
  </style>
</head>
<body>
  <h1>ESP32 Actuator Control - No Scripts</h1>

  <!-- Extend Actuator 1 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="EXTEND">
    <input type="hidden" name="actuator" value="1">
    <button type="submit" class="button">Extend Actuator 1</button>
  </form>

  <!-- Retract Actuator 1 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="RETRACT">
    <input type="hidden" name="actuator" value="1">
    <button type="submit" class="button">Retract Actuator 1</button>
  </form>

  <!-- Extend Actuator 2 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="EXTEND">
    <input type="hidden" name="actuator" value="2">
    <button type="submit" class="button">Extend Actuator 2</button>
  </form>

  <!-- Retract Actuator 2 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="RETRACT">
    <input type="hidden" name="actuator" value="2">
    <button type="submit" class="button">Retract Actuator 2</button>
  </form>

  <!-- Extend Actuator 3 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="EXTEND">
    <input type="hidden" name="actuator" value="3">
    <button type="submit" class="button">Extend Actuator 3</button>
  </form>

  <!-- Retract Actuator 3 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="RETRACT">
    <input type="hidden" name="actuator" value="3">
    <button type="submit" class="button">Retract Actuator 3</button>
  </form>

  <!-- Extend Actuator 4 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="EXTEND">
    <input type="hidden" name="actuator" value="4">
    <button type="submit" class="button">Extend Actuator 4</button>
  </form>

  <!-- Retract Actuator 4 -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="RETRACT">
    <input type="hidden" name="actuator" value="4">
    <button type="submit" class="button">Retract Actuator 4</button>
  </form>

  <!-- Extend All Actuators -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="EXTEND">
    <input type="hidden" name="actuator" value="0">
    <button type="submit" class="button">Extend All Actuators</button>
  </form>

  <!-- Retract All Actuators -->
  <form method="GET" action="/toggle">
    <input type="hidden" name="action" value="RETRACT">
    <input type="hidden" name="actuator" value="0">
    <button type="submit" class="button">Retract All Actuators</button>
  </form>

</body>
</html>

  )rawliteral";

  return html;
}


void setup() {
  Serial.begin(115200);      // Serial communication with the computer
  Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN); // Serial communication with Arduino Mega (RX, TX)

  pinMode(LED_BUILTIN, OUTPUT);  // Optional: Use built-in LED for testing

//  btManager.begin();
  wifiManager.connectToWiFi();
  webServerManager.begin();
  otaUpdater.beginOTA();

  /** Make it clear the esp32 was just started. **/
  Serial.print ("\n\n/**\n/**  ");
  Serial.println (ESP32ScriptVersion);
  Serial.println ("/**\n/**  ESP32 Started\n/**\n");
  /** Send a communication to Mega board so it knows we just started **/
  Serial2.println ("ESP32 Booted and Connected.");
}


/** Begin actual executable code **/
void loop() {
 // btManager.handleBluetooth();
  wifiManager.handleWiFi();
  webServerManager.handleClient();
  otaUpdater.handleOTA();
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