//
// Created by fredr on 3/20/2025.
//
// WebServerManager.cpp

#include "esp32/WebServerManager.h"

// Constructor: set up the server and default page content.
WebServerManager::WebServerManager() : server(80) {
  // Set initial page content. This can be later updated using updatePageContent.
  pageContent = "Windows Controller Interface";
  // Setup the root route ("/") to call the handleRoot member.
  server.on("/", [this]()
            { handleRoot(); });
}

// Begin the web server.
void WebServerManager::begin() {
  server.begin();
  Serial.println("Web Server started on port 80.");
}
// Called from the main loop: delegates to the underlying server to process requests.
void WebServerManager::handleClient() {
  server.handleClient();
}
// Updates the current page content and reassigns the root route.
void WebServerManager::updatePageContent(const String &newPageHTML) {
  pageContent = newPageHTML;
  // Re-setup the root route so that it serves the updated content.
  server.on("/", [this]()
            { handleRoot(); });
  Serial.println("Web page content updated.");
}

// Returns the current page HTML content.
String WebServerManager::generateHTML() {
  return pageContent;
}
// Root route handler: sends the pageContent as the HTTP response.
void WebServerManager::handleRoot() {
  server.send(200, "text/html", pageContent);
}
