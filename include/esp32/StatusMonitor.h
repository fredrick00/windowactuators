//
// Created by fredr on 3/14/2025.
//
// StatusMonitor.h
#pragma once
#include <Arduino.h>
#include "StatusReportProcessor.h"  // Ensure this header defines StatusReportData and StatusReportProcessor

namespace ActuatorsController {
class StatusMonitor {
public:
    // Constructor accepts a reference to a StatusReportProcessor
    explicit StatusMonitor(StatusReportProcessor &processor);
    // Checks Serial2 for data, processes it, and returns true if an update was performed.
    bool updateStatus();
    // Returns a formatted status string if available.
    String getFormattedStatus() const;
    // Return the status report.
    const StatusReportData& getStatusReport() const;

private:
    StatusReportProcessor &statusProcessor;
};
} // namespace ActuatorsController