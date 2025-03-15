//
// Created by fredr on 3/14/2025.
//
// WebController.h
#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>

// Forward-declare worker classes (to be defined elsewhere)
class ActuatorCommandExecutor; // Responsible for sending extend/retract/force commands
class ActuatorVisualizer; // Takes state data and produces HTML fragments for progress bars
class StatusReportProcessor; // Processes incoming status reports from the Mega

class ActuatorWebController { // Handles web requests and generates HTML pages
    public:
// The constructor receives references to helper objects that handle commands and visualization.
      ActuatorWebController(WebServer &srv, ActuatorCommandExecutor &cmdExec, ActuatorVisualizer &visualizer, StatusReportProcessor &statusProc);

        // Initializes web routes – called from setup()
        void initRoutes();
        // Call this in loop() regularly to handle client requests.
        void handleClient();
        // Generates the full dynamic HTML page.
        String generatePage();

    private:
      WebServer &server; // Reference to the web server (already instantiated)
      ActuatorCommandExecutor &cmdExec; // Handles sending actuator commands
      ActuatorVisualizer &visualizer; // Creates visual representations (progress bars, etc.)
      StatusReportProcessor &statusProc; // Processes incoming status reports from the Mega

      // Private route handler functions.
      void handleRoot();
      void handleActuatorCommand();
      void handleRefreshStatus();
    // Helper to build actuator control buttons (with progress bars)
      String buildActuatorControl(int index);
};
