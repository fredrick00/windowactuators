//
// Created by fredr on 3/17/2025.
//
// StatusReportFormatter.cpp

#include "esp32/StatusReportFormatter.h"
#include <string.h>  // For strcmp

namespace ActuatorsController {

StatusReportFormatter::StatusReportFormatter() {
    // Constructor implementation (if additional initialization is needed, add here)
}

StatusReportFormatter::~StatusReportFormatter() {
    // Destructor implementation (cleanup if needed)
}

String StatusReportFormatter::format(const StatusReportData &reportData) {
    String html;
    // We'll iterate through inputMappings to process unique actuator names that are not buttons.
    // This ensures that for each actuator (determined by actuatorName), we generate a control group.
    const size_t count = MAX_INPUTS_COUNT;
    // For simplicity we assume a small maximum number (here assumed 10) of unique actuator names.
    const char* processed[10];
    size_t processedCount = 0;

    for (size_t i = 0; i < count; ++i) {
        // Skip global button mappings.
        if (inputMappings[i].isButton) {
            continue;
        }
        // Check if we've already processed this actuatorName.
        bool alreadyProcessed = false;
        for (size_t j = 0; j < processedCount; ++j) {
            if (strcmp(processed[j], inputMappings[i].actuatorName) == 0) {
                alreadyProcessed = true;
                break;
            }
        }
        if (!alreadyProcessed) {
            // Mark this actuatorName as processed.
            processed[processedCount++] = inputMappings[i].actuatorName;
            // Build the control group for the actuator.
            // At this stage, formatActuatorControl() is a stub returning NULL.
            html += formatActuatorControl(inputMappings[i].actuatorName, reportData);
        }
    }
    return html;
}

int StatusReportFormatter::findMappingIndexByName(const char* actuatorName) const {
    // Iterate over the inputMappings array and return the index of the first matching actuatorName.
    for (size_t i = 0; i < MAX_INPUTS_COUNT; ++i) {
        if (strcmp(inputMappings[i].actuatorName, actuatorName) == 0) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

const InputMapping* const* StatusReportFormatter::findMappingPair(const char* actuatorName) const {
    // Look for a pair of mappings associated with the given actuatorName.
    // One with Mode::EXTENDING and one with Mode::RETRACTING.
    const InputMapping* extendMapping = nullptr;
    const InputMapping* retractMapping = nullptr;

    for (size_t i = 0; i < MAX_INPUTS_COUNT; ++i) {
        if (!inputMappings[i].isButton && strcmp(inputMappings[i].actuatorName, actuatorName) == 0) {
            if (inputMappings[i].mode == Mode::EXTENDING) {
                extendMapping = &inputMappings[i];
            } else if (inputMappings[i].mode == Mode::RETRACTING) {
                retractMapping = &inputMappings[i];
            }
        }
    }
    // Only return a valid pair if both exist.
    if (extendMapping && retractMapping) {
        // Use a static array to hold the pair pointers. (This is valid since we expect a small fixed size.)
        static const InputMapping* pair[2];
        pair[0] = extendMapping;
        pair[1] = retractMapping;
        return pair;
    }
    return nullptr;
}

} // namespace ActuatorsController
