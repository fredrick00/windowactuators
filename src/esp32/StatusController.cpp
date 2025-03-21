//
// Created by fredr on 3/16/2025.
// File: StatusController.cpp
// Description: Coordinates between StatusMonitor and WebPageBuilder.

#include "esp32/StatusController.h"

StatusController::StatusController(StatusMonitor &monitor, WebPageBuilder &webBuilder)
    : statusMonitor(monitor), webPageBuilder(webBuilder) { }

void StatusController::update() {
    // Check for status update
    if (statusMonitor.updateStatus()) {
        // Pass the updated status to the web page builder
        webPageBuilder.buildPage(statusMonitor.getStatusReport());
    }
}
