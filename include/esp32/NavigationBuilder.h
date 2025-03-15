//
// Created by fredr on 3/14/2025.
//
/* NavigationBuilder.h */
#pragma once
#include <Arduino.h>

class NavigationBuilder {
    public: NavigationBuilder();
    virtual ~NavigationBuilder();
    // Add a navigation item (for example, a link with display text).

    void addNavItem(const String &displayText, const String &url);
    // Clear all navigation items.
    void clearNavItems();
    // Build and return the complete HTML <nav> structure.
    virtual String buildNavigation();

protected:
    // A simple structure to hold individual navigation items.
    struct NavItem {
      String text;
      String link;
    };
// We store the navigation items in an array-like structure.
    // For simplicity on Arduino, we're using a fixed array.
    // Adjust MAX_ITEMS as needed.
    static const int MAX_ITEMS = 10;
    NavItem navItems[MAX_ITEMS];
    int itemCount;
    // Helper to generate a list of navigation items.
    virtual String buildNavList();
};