//
// Created by fredr on 3/14/2025.
//
/* esp32_main.cpp */
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include "StatusReportFormatter.h"

namespace ActuatorsController {

  class StatusReportProcessor {
  public:
    // Constructor: takes a reference to an input Stream (e.g., Serial2)
    StatusReportProcessor(Stream &inputStream) : inStream(inputStream), report {} {}
    // Process incoming JSON data. If a complete report is read, fills in 'report'
    // and returns true; otherwise returns false.
    bool process(Stream &dataStream) {
      if (inStream.available()) {
        // Read until newline assuming the JSON string is terminated by '\n'
        String jsonReport = inStream.readStringUntil('\n');
        return parseReport(jsonReport, report);
      }
      return false;
    }


    // getReport simply returns a constant reference to the internal
    // report data structure.
    const StatusReportData &getReport() const {
      return report;
    }


    static void printReport(const StatusReportData &report) {
      Serial.print("Timestamp: ");
      Serial.println(report.timestamp);
      Serial.print("Force mode: ");
      Serial.println(report.forceMode ? "true" : "false");
      for (uint8_t i = 0; i < report.actuatorCount; i++) {
        const ActuatorData &act = report.actuators[i];
        Serial.print("Actuator ");
        Serial.print(act.index);
        Serial.print(": active=");
        Serial.print(act.active ? "true" : "false");
        Serial.print(", mode=");
        Serial.print(act.mode);
        Serial.print(", position=");
        Serial.print(act.position);
        Serial.print(", maxDuration=");
        Serial.println(act.maxDuration);
      }
    }

  private:
    Stream &inStream;
    StatusReportData report;
    // Parses the JSON report from the given String and populates 'report'.
    // Returns true if parsing was successful; false otherwise.
    bool parseReport(const String &jsonReport, StatusReportData &report) {
      StaticJsonDocument<1024> doc;
      DeserializationError error = deserializeJson(doc, jsonReport);
      if (error) {
        Serial.print("Failed to parse status report: ");
        Serial.println(error.f_str());
        return false;
      }
      // Extract top-level data
      report.timestamp = doc["timestamp"] | 0;
      report.forceMode = doc["forceMode"] | false;
      // Initialize actuator count
      report.actuatorCount = 0;
      JsonArray actuatorsArray = doc["actuators"].as<JsonArray>();
      // Loop through the actuators, ensuring we do not exceed the maximum allowed.
      for (JsonObject actuator : actuatorsArray) {
        if (report.actuatorCount >= StatusReportData::MAX_ACTUATORS) {
          break;
        }
        ActuatorData &act = report.actuators[report.actuatorCount];
        act.index = actuator["index"] | 0;
        act.active = actuator["active"] | false;
        act.mode = actuator["mode"] | "";
        act.position = actuator["position"] | 0;
        act.maxDuration = actuator["maxDuration"] | 0;
        report.actuatorCount++;
      }
      return true;
    }

  };

} // namespace ActuatorsController