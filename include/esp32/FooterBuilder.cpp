//
// Created by fredr on 3/14/2025.
//
/* FooterBuilder.cpp */
#include "FooterBuilder.h"

    ////////////////////////////////////////////////////////////
    // Constructor & Destructor
    ////////////////////////////////////////////////////////////
FooterBuilder::FooterBuilder() {
  footerContent = "";
}
    // Initialize with a default footer content, if desired.
FooterBuilder::~FooterBuilder() { }
    // Cleanup if any resources were allocated.
    ////////////////////////////////////////////////////////////
    // Public Methods
    ////////////////////////////////////////////////////////////
void FooterBuilder::setFooterText(const String &text) {
    // Set the internal footer content.
    footerContent = text;
}
String FooterBuilder::buildFooter() {
    // Build the final HTML for the footer.
       String footerHTML;
       footerHTML += " \n";
       footerHTML += footerContent + "\n";
       footerHTML += " \n";
       return footerHTML;
}