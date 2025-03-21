//
// Created by fredr on 3/17/2025.
//
// StatusReportFormatter.h
#pragma once

#include <Arduino.h>
#include "StatusReportProcessor.h"  // Contains StatusReportData
#include "inputmapping.h"           // Contains InputMapping and inputMappings

namespace ActuatorsController {

// This class processes the status report from the Mega.
// Structure to hold individual actuator data.
    struct ActuatorData {
      int index;
      bool active;
      String mode;
      int position;
      int maxDuration;
    };

    // Structure to hold the overall report data; note we use a fixed‐size array.
    struct StatusReportData {
      unsigned long timestamp;
      bool forceMode;
      static const int MAX_ACTUATORS = 10;
      // maximum number of actuators
      ActuatorData actuators[MAX_ACTUATORS];
      uint8_t actuatorCount;
      // creating a variable to hold the status message.
      String statusMessage;
    };

class StatusReportFormatter {
public:
    // Constructor
    StatusReportFormatter();

    // Virtual destructor in case of future subclassing.
    virtual ~StatusReportFormatter();

    // Formats the provided StatusReportData into an HTML snippet.
    // The formatted HTML includes paired actuator control groups using
    // the actuatorName as the title for the button groups.
    virtual String format(const StatusReportData &reportData);

private:
    // Finds the first index in inputMappings with a matching actuatorName.
    // Returns -1 if not found.
    int findMappingIndexByName(const char* actuatorName) const;

    // Given an actuator name, finds and returns a pointer to the pair of mappings
    // (one for EXTENDING and one for RETRACTING). If either mapping is missing,
    // returns nullptr.
    const InputMapping* const* findMappingPair(const char* actuatorName) const;

    // Builds and returns the HTML string for a paired button set using the provided
    // extend and retract mappings from inputMappings. The actuator's status information
    // from StatusReportData may be used to further customize the output.
    String buildButtonPair(const InputMapping &extendMapping,
                           const InputMapping &retractMapping,
                           const StatusReportData &reportData) const;

    // Formats the control group for a given actuator. This function extracts the
    // necessary data from StatusReportData and pairs the controls based on actuatorName.
    String formatActuatorControl(const char* actuatorName,
                                 const StatusReportData &reportData) const;
};

} // namespace ActuatorsController