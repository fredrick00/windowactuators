//
// Created by fredr on 3/14/2025.
//
#include "esp32/WebPageBuilder.h"
#include <Arduino.h>
#include "esp32/HeaderBuilder.h"
#include "esp32/NavigationBuilder.h"
#include "esp32/FooterBuilder.h"
#include "esp32/BodyBuilder.h"

// Constructor: Initialize all properties to empty strings.
WebPageBuilder::WebPageBuilder(const String &defaultTitle)
    : pageTitle(defaultTitle), cssLink(""), jsLink(""), navigationContent(""), dynamicBodyContent("")
	{ }
    // Cleanup resources if needed.
	WebPageBuilder::~WebPageBuilder() { }
    // Setters for page properties.
    void WebPageBuilder::setPageTitle(const String &title) {
      pageTitle = title;
    }
    void WebPageBuilder::setCSSLink(const String &cssLinkValue) {
      cssLink = cssLinkValue;
    }
    void WebPageBuilder::setJSLink(const String &jsLinkValue) {
      jsLink = jsLinkValue;
    }
    void WebPageBuilder::setNavigation(const String &navContent) {
      navigationContent = navContent;
    }
    void WebPageBuilder::clearBodyContent() {
      dynamicBodyContent = "";
    }
    void WebPageBuilder::addBodyContent(const String &additionalContent) {
      dynamicBodyContent += additionalContent;
    }
    // Build and return the complete HTML page.
    String WebPageBuilder::buildPage(const StatusReportData &statusData) {
    String page;
    // Append header section.
    page += getHeader();
    // Append navigation if provided
    page += getNavigation();
    // Append dynamic body content
    page += getBody(statusData);
    // Append footer
    page += getFooter();

    return page;

  }
  // Build the section: includes meta tags, title, and CSS/JS links.
  String WebPageBuilder::getHeader() {
      headerBuilder.setTitle(pageTitle);
	  return headerBuilder.buildHeader();
  }
// Build the navigation section.
String WebPageBuilder::getNavigation() {
  String nav = navigationBuilder.buildNavigation();
  return nav;
}
// Build the dynamic body content section.
String WebPageBuilder::getBody(const StatusReportData &statusData) {
  String body = bodyBuilder.buildBody();
  return body;
}
// Build the footer section.
String WebPageBuilder::getFooter() {
  String footer = footerBuilder.buildFooter();
    return footer;
}

