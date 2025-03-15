//
// Created by fredr on 3/14/2025.
//
// StatusMonitor.cpp

#include "StatusMonitor.h"
#include "StatusReportProcessor.h"  // Included here if additional StatusReportProcessor details are needed
#include <Arduino.h>

// Constructor: initializes the reference to the status processor.
StatusMonitor::StatusMonitor(StatusReportProcessor &processor)
    : statusProcessor(processor) {
    // You can also initialize Serial2 here if needed, though typically that's done in setup().
}

// updateStatus:
// Checks Serial2 for available data. If data is available, it reads input until newline,
// parses the input into a StatusReportData object, and triggers the processor update.
// Returns true if processing was successful.
bool StatusMonitor::updateStatus() {
    if (Serial2.available() > 0) {
        // Read from Serial2 until newline; adjust delimiter if necessary.
        String input = Serial2.readStringUntil('\n');

        // Create and populate the StatusReportData instance.
        // Adapt the following parsing logic to fit your report structure.
        StatusReportData report;
        // Example: Assume the report has a field called 'value' that accepts an integer.
        report.value = input.toInt();

        // Process the report using the injected status processor.
        // Returns true if the process was successful.
        return statusProcessor.process(report);
    }
    return false; // No data available to process.
}

// getFormattedStatus:
// Retrieves a formatted version of the current status data.
// This implementation assumes that your StatusReportProcessor has a method to return the status as a String.
// If not, you can format the data here.
String StatusMonitor::getFormattedStatus() const {
    // Example: If the processor exposes a method getFormattedStatus(), use it.
    // Otherwise, format the data stored in the processor or another internal data member.
    return statusProcessor.getFormattedStatus();
}