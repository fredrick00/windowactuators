//
// Created by fredr on 3/14/2025.
//
/* HeaderBuilder.h */

#pragma once
#include <Arduino.h>

class HeaderBuilder {
    public:
      HeaderBuilder();
        virtual ~HeaderBuilder();
        // Set the title for the page header.
        void setTitle(const String &title);

        // Optionally set a link to a CSS stylesheet.
        void setCSSLink(const String &cssLink);

        // Optionally set a link to a JavaScript file.
        void setJSLink(const String &jsLink);

        // Build and return the complete HTML <head> section.
        // This includes meta tags, title, and any provided CSS/JS links.
        virtual String buildHeader();

    protected:
         String pageTitle;
         String stylesheetLink;
         String javascriptLink;


        // Helper for generating common meta tags.
        virtual String getMetaTags();

};

