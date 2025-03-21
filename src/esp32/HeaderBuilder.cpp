//
// Created by fredr on 3/14/2025.
//
/* HeaderBuilder.cpp */

#include "esp32/HeaderBuilder.h"

//////////////////////////////////////////////////////////// // Constructor & Destructor ////////////////////////////////////////////////////////////
HeaderBuilder::HeaderBuilder() {} // You can initialize default values if needed.
HeaderBuilder::~HeaderBuilder() {} // Clean-up if necessary.

//////////////////////////////////////////////////////////// // Setter Methods ////////////////////////////////////////////////////////////
void HeaderBuilder::setTitle(const String &title) { pageTitle = title; }
void HeaderBuilder::setCSSLink(const String &cssLink) { stylesheetLink = cssLink; }
void HeaderBuilder::setJSLink(const String &jsLink) { javascriptLink = jsLink; }

//////////////////////////////////////////////////////////// // Protected Helper ////////////////////////////////////////////////////////////
String HeaderBuilder::getMetaTags() {
	String metaTags;
	// Common meta tags.
    metaTags += "<meta charset='UTF-8'>\n";
    metaTags += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
    return metaTags;
}

//////////////////////////////////////////////////////////// // Build Header Section ////////////////////////////////////////////////////////////
String HeaderBuilder::buildHeader() {
    // Start with DOCTYPE and opening <html> tag.
  String header = "<!DOCTYPE html>\n";
    header += "<html>\n";
  header += "\n";
    header += "<head>\n";
    // Append meta tags.
    header += getMetaTags();
    // Append title.
    if (pageTitle.length() > 0) {
      header += "<title>" + pageTitle + "</title>\n";
    } else {
      header += "<title>No Title Provided</title>\n";
    }
    // Append stylesheet link if provided.
    if (stylesheetLink.length() > 0) {
      header += "<link rel='stylesheet' type='text/css' href='" + stylesheetLink + "'>\n";
    }
    // Append JavaScript link if provided.
    if (javascriptLink.length() > 0) {
      header += "<script src='" + javascriptLink + "'></script>\n";
    }
    header += "</head>\n";

  return header;
}