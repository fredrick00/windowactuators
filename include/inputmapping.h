//
// Created by fredr on 3/9/2025.
//

#pragma once

// inputmapping.h
#include "Debounced.h"
#include <array>


namespace ActuatorsController {

    // Enum for specifying the switch mode.
    enum class Mode {
        NONE,
        EXTENDING,
        RETRACTING,
        PAUSED
    };

    // Struct that represents the mapping for an input.
    struct InputMapping {
        const char* actuatorName; // Give a title to the actuator.
        int inputPin;        // Physical input pin number.
        int actuatorPin;   // Index of the actuator; use -1 for global controls.
        Mode mode;     // The mode, such as EXTEND or RETRACT.
        bool isButton;       // True if this mapping represents a global button.
    };


// Constant array of input mappings
    constexpr InputMapping inputMappings[] = {
        // Extend mappings for switches: uses first half of relayPins: {51, 49, 47, 45}
        {"Actuator 1", 8, 51, Mode::EXTENDING, false},
        {"Actuator 2", 7, 49, Mode::EXTENDING, false},
        {"Actuator 3", 5, 47, Mode::EXTENDING, false},
        {"Actuator 4", 3, 45, Mode::EXTENDING, false},

        // Retract mappings for switches: uses second half of relayPins: {"Actuator ", 43, 41, 39, 37}
        {"Actuator 1", 9, 43, Mode::RETRACTING, false},
        {"Actuator 2", 6, 41, Mode::RETRACTING, false},
        {"Actuator 3", 4, 39, Mode::RETRACTING, false},
        {"Actuator 4", 2, 37, Mode::RETRACTING, false},

        {"All Actuators", 12, -1, Mode::EXTENDING, true},
        {"All Actuators", 13, -1, Mode::RETRACTING, true}

    };
    // Compute the total count at compile-time.
    constexpr size_t MAX_INPUTS_COUNT = sizeof(inputMappings) / sizeof(inputMappings[0]);

    constexpr size_t EXTEND_BUTTON_INDEX = MAX_INPUTS_COUNT - 2;
    constexpr size_t RETRACT_BUTTON_INDEX = MAX_INPUTS_COUNT - 1;
    // Define MAX_PINS as (count - 2 to exclude extend/retract all).
    constexpr int MAX_PINS = MAX_INPUTS_COUNT - 2;
    constexpr int TOTAL_ACTUATORS = MAX_PINS;

    // Using std::array for fixed-size allocation.
    std::array<Debounced, MAX_INPUTS_COUNT> debouncedSwitches;

    // Create an instance (adjust the pin and interval as needed)
    Debounced mySwitch(2, 50); // Pin 2 with 50ms debounce time

} // namespace ActuatorsController
