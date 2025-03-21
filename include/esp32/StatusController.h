//
// Created by fredr on 3/16/2025.
// File: StatusController.h
// Description: Provides interface for the status controller.

#pragma once
#include "StatusMonitor.h"
#include "WebPageBuilder.h"

class StatusController {
public:
    StatusController(StatusMonitor &monitor, WebPageBuilder &webBuilder);
    // Call this method continually (e.g., inside the main loop)
    void update();

private:
    StatusMonitor &statusMonitor;
    WebPageBuilder &webPageBuilder;
};
