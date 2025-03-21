//
// Created by fredr on 3/14/2025.
//
#include "esp32/BodyBuilder.h"
#include "esp32/StatusReportProcessor.h"
using namespace ActuatorsController;

    //
    // Constructor: Initializes an empty body.
    BodyBuilder::BodyBuilder() { bodyContent = ""; }
    //
    // Virtual destructor
    BodyBuilder::~BodyBuilder() {}
    // Cleanup, if necessary.
    //
    // Clears the current body content.
    void BodyBuilder::clearBody() { bodyContent = ""; }
    //
    // Appends additional content (raw HTML) to the body.
    void BodyBuilder::addContent(const String &content) { bodyContent += content; }
    //
    // Convenience method: Adds a control button for a given window.
    //
    // windowId: identifier for the window (or button group)
    // action: the control command (e.g., "extend", "retract", "forceExtend", "forceRetract", or "pause")
    // label: text to display on the button.
    // colorStyle: a CSS style string or a class name to color-code the button.
    void BodyBuilder::addWindowControlButton(const String &windowId, const String &action, const String &label, const String &colorStyle) {
    // Build a button element with an onClick event that calls a JavaScript function.
    //String button = "<button id=" + windowId + "_" + action + "";
    String button = "<button id=\"" + windowId + "_" + action + " ";
    // If colorStyle represents a CSS class, use class attribute. Adjust as needed.
    button += "class=\"" + colorStyle + "\" ";
    // The onClick calls a JavaScript function (handleWindowAction) with the window id and action.
    button += "onclick=\"handleWindowAction('" + windowId + "', '" + action + "')\">";
    // Button label text.
    button += label;
    // Close the button element.
    button += "</button>\n";
    // Append the generated button to the body content.
    bodyContent += button;
}
//
    // Builds and returns the complete HTML for the body section.
     String BodyBuilder::buildBody() {
       String html = "\n<body>\n";
       html += bodyContent;
       html += "\n</body>\n";
       return html;
     }

 // buildBody() overload for processing StatusReportData.
String BodyBuilder::buildBody(const StatusReportData &statusReport) {
  String html = "\n<body>\n";
  //
  html += "<div class=\"status\">Status: " + statusReport.statusMessage + "</div>\n";
  // Append any additional content accumulated.
  html += bodyContent;
  html += "\n</body>\n";
  return html;
}