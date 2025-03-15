//
// Created by fredr on 3/14/2025.
//
/* FooterBuilder.h */
#pragma once
#include <Arduino.h>

class FooterBuilder {
    public:
      FooterBuilder();
      virtual ~FooterBuilder();
    // Set the footer text or HTML content.
      void setFooterText(const String &text);
    // Build and return the complete footer HTML.
      virtual String buildFooter();
protected:
  String footerContent;
};
