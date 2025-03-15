//
// Created by fredr on 3/14/2025.
//
// StatusMonitor.h
#ifndef STATUSMONITOR_H
#define STATUSMONITOR_H

#include <Arduino.h>
#include "StatusReportProcessor.h"  // Ensure this header defines StatusReportData and StatusReportProcessor

class StatusMonitor {
public:
    // Constructor accepts a reference to a StatusReportProcessor
    explicit StatusMonitor(StatusReportProcessor &processor);

    // Checks Serial2 for data, processes it, and returns true if an update was performed.
    bool updateStatus();

    // Returns a formatted status string if available.
    String getFormattedStatus() const;

private:
    StatusReportProcessor &statusProcessor;
};

#endif // STATUSMONITOR_H