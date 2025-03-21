//
// Created by fredr on 3/14/2025.
//
/* WebPageBuilder.h */
#pragma once
#include <Arduino.h>
#include "StatusReportProcessor.h"
#include "HeaderBuilder.h"
#include "BodyBuilder.h"
#include "FooterBuilder.h"
#include "NavigationBuilder.h"

using namespace ActuatorsController;

class WebPageBuilder {
  public:
    WebPageBuilder(const String &defaultTitle = "Windows Controller Interface");
    virtual ~WebPageBuilder();
    // Setters for page properties.
    void setPageTitle(const String &title);
    void setCSSLink(const String &cssLink);
    void setJSLink(const String &jsLink);
    // Set or update the navigation bar content.
    void setNavigation(const String &navContent);
    // Clear any dynamic additions to the body.
    void clearBodyContent();
    // Append extra content to the body; allows for multiple calls that accumulate content.
    void addBodyContent(const String &additionalContent);
    // Build and return the complete HTML page.
    // This method calls individual functions to generate parts of the page.
    virtual String buildPage(const StatusReportData &statusData);

protected:
    // Returns the section including the title and links
    virtual String getHeader();
    // Returns the navigation bar section
    virtual String getNavigation();
    // Returns the complete dynamic content/body of the page
    virtual String getBody(const StatusReportData &statusData);
    // Returns the footer part
    virtual String getFooter();
    // Utility to generate common meta tags (optional)
    virtual String getMetaTags();
    // Page properties and dynamic content.
    String pageTitle;
    String cssLink;
    String jsLink;
    String navigationContent;
    String dynamicBodyContent;

    // Presistent web page building instances
    HeaderBuilder headerBuilder;
    BodyBuilder bodyBuilder;
    FooterBuilder footerBuilder;
    NavigationBuilder navigationBuilder;

};
