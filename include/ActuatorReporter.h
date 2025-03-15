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
    String generateReport() const {
        String report = "{ \"timestamp\": " + String(millis());
        report += ", \"forceMode\": " + String(relays.isForceMode() ? "true" : "false"); // Report forced mode
        report += ", \"actuators\": [";
        for (int i = 0; i < MAX_PINS; i++) {
            const auto &state = relays.relayStates[i];
            report += "{ \"index\": " + String(i);
            report += ", \"active\": " + String(state.isActive ? "true" : "false");
            report += R"(, "mode": ")";
            if (state.isActive) {


                report += (state.relayState != Mode::NONE ? state.relayState == Mode::EXTENDING ? "EXTENDING" : "RETRACTING" : "IDLE");
            } else {
                report += "IDLE";
            }
            report += "\", \"position\": " + String(state.actuatorPosition);
            report += ", \"maxDuration\": " + String(state.maxDuration);
            report += "}";
            if (i < MAX_PINS - 1) {
                report += ", ";
            }
        }
        report += "] }";
        return report;
    }

    // Sends the report via Serial2 (assumed to be used for communication with the ESP32).
    void sendStatusReport() const {
        String reportString = generateReport();
        Serial2.println(reportString);
        // DEBUG output
        Serial.print (" Mega Local Output:\n");
        Serial.println(reportString);
    }

    // Call this function periodically (e.g., from loop()).
    // It will send a new report if the reporting interval has elapsed.
    void update() {
        unsigned long currentTime = millis();
        if ((currentTime - lastReportTime) >= REPORT_INTERVAL) {
            sendStatusReport();
            lastReportTime = currentTime;
        }
    }

private:
    // Reference to the MegaRelayControl instance, from which we retrieve actuator states.
    MegaRelayControl &relays;
    // Timestamp of the last report sent.
    unsigned long lastReportTime;
    // Reporting interval in milliseconds (adjust as needed).
    static const unsigned long REPORT_INTERVAL = 1000UL;
};
} // namespace ActuatorsController
