//
// Created by fredr on 3/14/2025.
//
#include "WebPageBuilder.h"
#include <Arduino.h>
#include "HeaderBuilder.h"
#include "NavigationBuilder.h"
#include "FooterBuilder.h"
#include "BodyBuilder.h"

// Constructor: Initialize all properties to empty strings.
WebPageBuilder::WebPageBuilder() {
    pageTitle = "";
    cssLink = "";
    jsLink = "";
    navigationContent = "";
    dynamicBodyContent = "";
}
    // Virtual destructor.
    WebPageBuilder::~WebPageBuilder() {}
    // Cleanup resources if needed. }
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
    String WebPageBuilder::buildPage() {
    String page;
    // Start with DOCTYPE and opening <html> tag.
    page += "<!DOCTYPE html>\n";
    page += "<html>\n";
    // Append header section.
    page += getHeader();
    // Start body section
    page += "<body>\n";
    // Append navigation if provided
    page += getNavigation();
    // Append dynamic body content
    page += getBody();
    // Append footer
    page += getFooter();
    // Close body and html tags
    page += "</body>\n";
    page += "</html>\n";
    return page;

  }
  // Build the section: includes meta tags, title, and CSS/JS links.
  String WebPageBuilder::getHeader() {
  String header;
    header += "<head>\n";
    // Append meta tags.
    header += getMetaTags();
    // Append title.
    if (pageTitle.length() > 0) {
      header += "<title>" + pageTitle + "</title>\n";
    } else { header += "<title>Default Title</title>\n";
    }
    // Append stylesheet link if provided.
    if (cssLink.length() > 0) {
      header += "<link rel='stylesheet' type='text/css' href='" + cssLink + "'>\n";
    }
    // Append JavaScript link if provided.
    if (jsLink.length() > 0) {
      header += "<script src='" + jsLink + "'></script>\n";
    } header += "</head>\n";
    return header;

}
// Build the navigation section.
String WebPageBuilder::getNavigation() {
  String nav;
    if (navigationContent.length() > 0)
  {
    nav += "<nav>\n";
    nav += navigationContent;
    nav += "\n</nav>\n";
  }
return nav;

}
// Build the dynamic body content section.
String WebPageBuilder::getBody() {
  String body;
    if (dynamicBodyContent.length() > 0)
    {
      body += "<div id='content'>\n";
      body += dynamicBodyContent;
      body += "\n</div>\n";
    }
    return body;

}
// Build the footer section.
String WebPageBuilder::getFooter() {
  String footer;
    footer += "<footer>\n";
    footer += " <p>Default Footer Content</p>\n";
    footer += "</footer>\n";
    return footer;
}

// Generate common meta tags.
String WebPageBuilder::getMetaTags() {
  String meta;
    meta += "<meta charset='UTF-8'>\n";
    meta += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>\n";
    return meta;

}
