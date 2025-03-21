//
// Created by fredr on 3/20/2025.
//
// WebServerManager.h
#pragma once

#include <Arduino.h>
#include <WebServer.h>

class WebServerManager {
  public:
    // Constructor initializes the web server on port 80.
    WebServerManager();
    // Starts the web server.
    void begin();
    // Should be called repeatedly from the main loop to process requests.
    void handleClient();
    // Update the HTML page content and reset the root route handler.
    void updatePageContent(const String &pageHTML);
    // Returns the current HTML page content.
    String generateHTML();
  private: // Underlying web server instance.
    WebServer server;
    // The HTML content to be served at the root.
    String pageContent;
    // Root route handler that sends back the current pageContent.
    void handleRoot();
};

