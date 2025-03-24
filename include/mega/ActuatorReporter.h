//
// Created by fredr on 3/14/2025.
//
#pragma once
#include <Arduino.h>
#include "MegaRelayControl.h"



namespace ActuatorsController {

class ActuatorReporter {
public:
    // Constructor: stores a reference to the MegaRelayControl which holds relay and state information.
    ActuatorReporter(MegaRelayControl &relayControl)
        : relays(relayControl), lastReportTime(millis()) {}

    // Virtual destructor (if you later subclass this reporter)
    virtual ~ActuatorReporter() = default;

    // Generates a JSON-formatted report string with current actuator states.
    String generateReport(int actuatorIndex) const {
        relays.relayStates[actuatorIndex].stateHasChanged = false;
        // we start in an array in case we choose to send multiple
        // entries in the future.
        String report = "{\"actuators\": [ {";
        report += "\"actuatorCount\": 1"; // it's always 1 actuator for now
        report += "\"timestamp\": " + String(millis());
        report += ", \"forceMode\": " + String(relays.isForceMode() ? "true" : "false"); // Report forced mode
        // reset stateHasChanged now that reporting has been called
        const auto &state = relays.relayStates[actuatorIndex];
        report += ", \"index\": " + String(actuatorIndex);
        report += ", \"active\": " + String(state.isActive ? "true" : "false");
        report += ", \"actuatorName\": \"" + state.actuatorName + "\"";
        report += R"(, "mode": ")";
        if (state.isActive) {
            report += (state.relayState != Mode::NONE ? state.relayState == Mode::EXTENDING ? "EXTENDING" : "RETRACTING" : "IDLE");
        } else {
            report += "IDLE";
        }
        report += "\", \"position\": " + String(state.actuatorPosition);
        report += ", \"maxDuration\": " + String(state.maxDuration);
        report += "}";

        report += " ] }";
        return report;
    }

    // Sends the report via Serial2 (assumed to be used for communication with the ESP32).
    void sendStatusReport(int actuatorIndex) const {
        String reportString = generateReport(actuatorIndex);
        // Send the json prepared report encapsulated with payload data.
        String encappedReport = encapsulateReport(reportString);
        Serial2.println(encappedReport);
        // DEBUG output
        Serial.print (" Mega Local Output:\n");
        Serial.println(encappedReport);
        Serial.println ("End Output.\n");
    }

private:
    // Reference to the MegaRelayControl instance, from which we retrieve actuator states.
    MegaRelayControl &relays;
    // Timestamp of the last report sent.
    unsigned long lastReportTime;
    // Reporting interval in milliseconds (adjust as needed).
    static const unsigned long REPORT_INTERVAL = 1000UL;

    // encapsulate the report with a payload prediction so the receiver knows how much data to expect.
    String encapsulateReport(const String &jsonReport) const {
        // Determine the length of the JSON payload.
        unsigned int payloadLength = jsonReport.length();
        // Build an 8-character hexadecimal header.
        // For example, a payload length of 1234 becomes "000004D2".
        char header[9]; // 8 characters plus a null terminator.
        snprintf(header, sizeof(header), "%08X", payloadLength);
        // Return the header concatenated with the JSON report.
        return String(header) + jsonReport;
    }

};
} // namespace ActuatorsController
