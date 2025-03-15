//
// Created by fredr on 3/14/2025.
//
/* HeaderBuilder.cpp */
#pragma once

#include "HeaderBuilder.h"

//////////////////////////////////////////////////////////// // Constructor & Destructor ////////////////////////////////////////////////////////////
HeaderBuilder::HeaderBuilder() {} // You can initialize default values if needed.
HeaderBuilder::~HeaderBuilder() {} // Clean-up if necessary.

//////////////////////////////////////////////////////////// // Setter Methods ////////////////////////////////////////////////////////////
void HeaderBuilder::setTitle(const String &title) { pageTitle = title; }
void HeaderBuilder::setCSSLink(const String &cssLink) { stylesheetLink = cssLink; }
void HeaderBuilder::setJSLink(const String &jsLink) { javascriptLink = jsLink; }

//////////////////////////////////////////////////////////// // Protected Helper ////////////////////////////////////////////////////////////
String HeaderBuilder::getMetaTags() { String metaTags;
// Common meta tags. metaTags += " \n"; metaTags += " \n";
// Feel free to add or override meta tags here if needed.
return metaTags; }

//////////////////////////////////////////////////////////// // Build Header Section ////////////////////////////////////////////////////////////
String HeaderBuilder::buildHeader() {
  String header;
  header += "\n";
  // Append the meta tags.
  header += getMetaTags();
  // Append the title if it's been set.
  if (pageTitle.length() > 0)
  {
    header += " " + pageTitle + "\n";
  }
  // Append the stylesheet link if provided.
  if (stylesheetLink.length() > 0) {
    header += " \n";
  }
  // Append the JavaScript link if provided.
  if (javascriptLink.length() > 0) {
    header += " \n";
  }
  header += "\n";
  return header;
}
