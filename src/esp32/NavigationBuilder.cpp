//
// Created by fredr on 3/14/2025.
//
/* NavigationBuilder.cpp */
#include "esp32/NavigationBuilder.h"

//////////////////////////////////////////////////////////// // Constructor & Destructor ////////////////////////////////////////////////////////////
NavigationBuilder::NavigationBuilder() : itemCount(0) {}// Initialize the navigation items count to 0.
NavigationBuilder::~NavigationBuilder() {} // Clean-up code, if needed in the future.

//////////////////////////////////////////////////////////// // Public Methods ////////////////////////////////////////////////////////////
void NavigationBuilder::addNavItem(const String &displayText, const String &url) { // Only add the item if there is space left in our fixed array.
  if (itemCount < MAX_ITEMS) {
    navItems[itemCount].text = displayText;
    navItems[itemCount].link = url;
    itemCount++;
    }
}

void NavigationBuilder::clearNavItems() { itemCount = 0; } // Reset the count to 0 to remove all items.
String NavigationBuilder::buildNavigation() {
  String navigation = buildNavList();
  if (navigation.length() == 0) {
    navigation = "\n<nav>\n" + navigation + " \n</nav>\n";
  }
  return navigation;
}

//////////////////////////////////////////////////////////// // Protected Helpers ////////////////////////////////////////////////////////////
String NavigationBuilder::buildNavList() {
  String listHTML;
  listHTML += " \n"; // Loop through each nav item to create an HTML list element.
  for (int i = 0; i < itemCount; i++) {
    listHTML += " - [" + navItems[i].text + "]() \n";
  }
  listHTML += " \n";
  return listHTML;
}
