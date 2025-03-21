//
// Created by fredr on 3/14/2025.
//
// StatusMonitor.cpp

#include "esp32/StatusMonitor.h"
#include "esp32/StatusReportProcessor.h"  // Included here if additional StatusReportProcessor details are needed
#include <Arduino.h>

namespace ActuatorsController {

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
    // Let the StatusReportProcessor read from Serial2, parse the JSON,
    // and update its internal StatusReportData structure.
    return statusProcessor.process(Serial2);
  }
  return false;
}

// getFormattedStatus:
// Retrieves a formatted version of the current status data.
// This implementation assumes that your StatusReportProcessor has a method to return the status as a String.
// If not, you can format the data here.

String StatusMonitor::getFormattedStatus() const {
  // Create a formatter instance to build the formatted HTML snippet.
  StatusReportFormatter formatter;
  // Use the formatter to format the current StatusReportData from the processor.
  return formatter.format(statusProcessor.getReport());
}

// getStatusReport: returns the current status report in StatusReportData format.
StatusReportData StatusMonitor::getStatusReport() const {
  return statusProcessor.getReport();
}



const StatusReportData &StatusMonitor::getStatusReport() const {
  return statusProcessor.getReport();
}

} // namespace ActuatorsController