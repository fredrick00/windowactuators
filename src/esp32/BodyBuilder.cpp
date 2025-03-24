//
// Created by fredr on 3/14/2025.
//
#include <vector>
#include "esp32/BodyBuilder.h"
#include "esp32/StatusReportProcessor.h"
#include "esp32/StatusReportFormatter.h"
#include "esp32/NavigationBuilder.h"
#include "esp32/WebPageBuilder.h"
// include inputmapping from Mega side of the project as we are generating web equivalent for switches and buttons.
#include "mega/inputmapping.h"

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
    //default buildBody handler
    String BodyBuilder::buildBody() { return bodyContent;}
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


 // buildBody() overload for processing StatusReportData.
String BodyBuilder::buildBody(const StatusReportData &statusReport) {
  	String html = "";
    // Open body tag.
    html += "\n<body>\n";

    // Append any pre-existing content (for example, control buttons).
    html += bodyContent;
    // Optionally, append additional control buttons if needed.
    html += buildControlButtons(statusReport);

    // Close body tag.
    html += "\n</body>\n";

    return html;
}


// -----------------------------------------------------------------------------
// Helper: Returns the CSS class for the button frame based on the desired
//  	button action and the current live mode in the actuator data.
//   - For the "extend" button, if the actuator’s mode is "extending", return a
//     light highlight; if it is "retracting", return a dark highlight.
//   - For the "retract" button, do the opposite.
//   - In all other cases, return a neutral styling.
// (You can later define the actual CSS for these classes.)
static String getFrameClass(const String &buttonAction, const String &currentMode) {
    if (buttonAction == "extend") {
        if (currentMode == "extending")
            return "highlight-light";
        else if (currentMode == "retracting")
            return "highlight-dark";
    } else if (buttonAction == "retract") {
        if (currentMode == "retracting")
            return "highlight-light";
        else if (currentMode == "extending")
            return "highlight-dark";
    }
    return "highlight-neutral";
}

// -----------------------------------------------------------------------------
// Helper: Constructs the HTML for one control button.
// The actual button is always rendered in its neutral base style (green for extend,
// red for retract). An outer wrapping div is given the highlight class as determined
// from the live mode.
static String buildButton(const String &actuatorName, const String &action, const String &currentMode) {
    String btnHtml;
    String frameClass = getFrameClass(action, currentMode);
    btnHtml += "<div class='button-frame " + frameClass + "'>";
    btnHtml += "<button id='" + actuatorName + "_" + action + "' class='control-btn " +
               (action == "extend" ? "green" : "red") + "' ";
    btnHtml += "onclick=\"handleWindowAction('" + actuatorName + "', '" + action + "')\">";
    btnHtml += (action == "extend" ? "Extend" : "Retract");
    btnHtml += "</button>";
    btnHtml += "</div>\n";
    return btnHtml;
}


// -----------------------------------------------------------------------------
// Helper: Builds the control group for an individual actuator using its live data.
// The actuator name is generated as "Actuator <index>".
static String formatActuatorControl(const ActuatorData &actuator) {
    String html;
    String actuatorName = "Actuator " + String(actuator.index);
    html += "<div class='control-group'>\n";
    html += "<h3>" + actuatorName + "</h3>\n";
    // Create the Extend and Retract buttons, using the current actuator mode
    // to decide on the highlighting.
    html += buildButton(actuatorName, "extend", actuator.mode);
    html += buildButton(actuatorName, "retract", actuator.mode);
    html += "</div>\n";
    return html;
}

// -----------------------------------------------------------------------------
// Helper: Builds the controls for "All Actuators".
// Here you might aggregate live data for all actuators; for now, we assume a
// neutral mode so the buttons are not highlighted.
static String formatAllActuatorsControl(const StatusReportData &report) {
    String html;
    String actuatorName = "All Actuators";
    // In a real implementation you might compute an overall mode from report data.
    String overallMode = "neutral";
    html += "<div class='control-group'>\n";
    html += "<h3>" + actuatorName + "</h3>\n";
    html += buildButton(actuatorName, "extend", overallMode);
    html += buildButton(actuatorName, "retract", overallMode);
    html += "</div>\n";
    return html;
}

// -----------------------------------------------------------------------------
// Revised buildControlButtons using StatusReportData.
// (Note: The function’s signature has been modified to accept live status data.)
// This function iterates over the live actuator data and produces a control group
// for each actuator, plus a separate control group for "All Actuators".
String BodyBuilder::buildControlButtons(const StatusReportData &statusReport) {
    String buttons;
    // Iterate through each actuator in the live status report.
    for (uint8_t i = 0; i < TOTAL_ACTUATORS; ++i) {
        buttons += formatActuatorControl(statusReport.actuators[i]);
    }
    // Add the control group for "All Actuators"
    buttons += formatAllActuatorsControl(statusReport);
    return buttons;
}

/** duplicate entry it looks like
// -----------------------------------------------------------------------------
// Example updated version of the buildBody overload that passes live
// status data to buildControlButtons.
String BodyBuilder::buildBody(const StatusReportData &statusReport) {
    String html = "<!DOCTYPE html>\n<html>\n";
    html += "\n<body>\n";
    html += "<div class='status'>Status: " + statusReport.statusMessage + "</div>\n";
    html += bodyContent;
    // Use the live data version of buildControlButtons.
    html += buildControlButtons(statusReport);
    html += "\n</body>\n";
    return html;
}
**/