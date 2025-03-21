//
// Created by fredr on 3/14/2025.
//
#pragma once
#include <Arduino.h>
#include "StatusReportProcessor.h"

using namespace ActuatorsController;

class BodyBuilder {
public:
    // Constructor: initializes an empty body.
    BodyBuilder();
    // Virtual destructor for proper cleanup in derived classes.
    virtual ~BodyBuilder();
    // Clears the current body content.
    void clearBody();
    // Appends additional content (raw HTML) to the body.
    void addContent(const String &content);
    // Convenience method: adds a control button for a given window.
    //
    // windowId: identifier for the window (or button group)
    // action: the control command (for example, "extend",
    // "retract", "forceExtend", "forceRetract", or "pause")
    // label: text to display on the button.
    // colorStyle: a CSS style string or a class name to color-code the button.
    void addWindowControlButton(const String &windowId, const String &action, const String &label, const String &colorStyle);
    // Builds and returns the complete HTML for the body section.
    virtual String buildBody();
    // Overloaded buildBody accepts StatusReportData.
    virtual String buildBody(const StatusReportData &statusReport);

protected:
    // Holds the dynamic content that will be wrapped in the body.
    String bodyContent;
    // New overload for buildControlButtons that accepts StatusReportData,
    // returning control buttons built from live data.
    String buildControlButtons(const StatusReportData &statusReport);

    };
