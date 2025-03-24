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
#include "esp32/StatusReportProcessor.h"
#include "esp32/WebPageBuilder.h"
#include "esp32/StatusMonitor.h"
#include "esp32/WebServerManager.h"

  using namespace ActuatorsController;

  const String ESP32ScriptVersion = "esp32WindowInterface v1.13";
  bool haveWiFi = true; // Boolean to toggle WiFi reconnect attempts

  unsigned long lastWiFiAttemptMillis = 0;
  const unsigned long wifiAttemptInterval = 60000; // 1 minute

  static unsigned long lastBluetoothCheck = 0;
  const unsigned long bluetoothCheckInterval = 1000; // Check for Bluetooth connection every second


  //BluetoothSerial SerialBT;

#define LED_BUILTIN 2 // Define LED_BUILTIN if it's not defined

  unsigned long lastStatusMillis = 0;
  unsigned long currentMillis = 0;
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
  StatusReportProcessor statusProcessor(Serial2);
  StatusMonitor statusMonitor(statusProcessor);
  WebPageBuilder webPageBuilder("Windows Controller Interface");


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


void loop() {
    currentMillis = millis();
    // Process any pending WiFi and client requests.
    wifiManager.handleWiFi();
    webServerManager.handleClient();
    otaUpdater.handleOTA();
    // Update status every statusInterval milliseconds.
    if (currentMillis - lastStatusMillis >= statusInterval) {
      lastStatusMillis = currentMillis;
      // Update the status from Serial2 data.
      if (statusMonitor.updateStatus()) {
        // Retrieve the structured status report
        const StatusReportData &statusData = statusMonitor.getStatusReport();
        // Build the HTML page using the updated status data.
        String pageHTML = webPageBuilder.buildPage(statusData);

/** begin debug **/
        if (&statusData != nullptr) {
          String out = "";
          out += String(statusData.timestamp) + ",";
          for (uint8_t i = 0; i < statusData.actuatorCount; i++) {
            // Assume each actuator provides a way to convert to a string.
            out += String(statusData.actuators[i].index) +  ": " + String(statusData.actuators[i].name);
            if (i < statusData.actuatorCount - 1) { out += ";"; }
          }
          out += "," + statusData.statusMessage;
          Serial.println(out);
        }

/** end debug **/
        // if (pageHTML.length() > 0) { Serial.println(pageHTML);}
        // Set or update the web server’s dynamic content.
        webServerManager.updatePageContent(pageHTML);
      }
    }
}


/** loop before statusProcessor
  void loop() {
    wifiManager.handleWiFi();
    webServerManager.handleClient();
    otaUpdater.handleOTA();
    if (Serial2.available()) {
      // Let the statusProcessor take care of processing the status.
      // This function should handle all the tokenization and
      // update actuatorStatus (or any other states) accordingly.
      if (Serial2.available()) {
        statusProcessor.process(Serial2);
      }
    }
  }
** ** End Loop before statusProcessor comment ** **/
  #endif