//
// Created by fredr on 3/14/2025.
//
#pragma once
#include <Arduino.h>
#include "MegaButton.h"
#include "inputmapping.h"
#include "MegaSwitch.h"

namespace ActuatorsController {

//-------------------------------------------------------------------- //
// MegaInputManager class: Reads two overall buttons and an array // of switch inputs (assumed here to be four physical switches).
class MegaInputManager {


private:
    static const int MAX_RELAY_PINS = ActuatorsController::MAX_RELAY_PINS;
    // Hardware buttons for overall extend/retract actions.
    unsigned long thisSwitchActivationTime[MAX_RELAY_PINS];
    unsigned long previousSwitchActivationTime[MAX_RELAY_PINS];




    public:
    // Use the same enum for button and switch events.
    MegaButton extendButton;
    MegaButton retractButton;
    MegaSwitch switches[MAX_RELAY_PINS]; // Use the same type as the events produced by buttons.
    ButtonState switchStates[MAX_RELAY_PINS];
    // State variables for the extend and retract buttons.
    ButtonState extendState;
    ButtonState retractState;

    // Constructor: Create the two buttons and initialize the switch array.
    MegaInputManager()
    : extendButton(ActuatorsController::inputMappings[EXTEND_BUTTON_INDEX].inputPin),
      retractButton(ActuatorsController::inputMappings[RETRACT_BUTTON_INDEX].inputPin),
      extendState(ButtonState::NONE),
      retractState(ButtonState::NONE)
    {
        // Initialize each physical switch with its corresponding pin.
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            switches[i] = MegaSwitch(ActuatorsController::inputMappings[i].inputPin);
            switchStates[i] = ButtonState::NONE;

            // timestamp for current and previous switch activation to detect FORCE extend/retract request.
            // for loop to cycle through switches and initiate thisSwitchActivationTime and previousSwitchActivationTime arrays to numSwitchPins in size.
            thisSwitchActivationTime[i] = 0;
            previousSwitchActivationTime[i] = 0;
        }

    }

    // Call this each loop to measure the current inputs.
    void updateInputs() {
        // Update the overall buttons.
        extendState = extendButton.getButtonState();
        retractState = retractButton.getButtonState();
        // Update each switch.
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            if (switches[i].hasStateChanged()) {
                // We treat a changed state that is pressed (LOW on Arduino when using INPUT_PULLUP) // as a SINGLE_PRESSED event.
                switchStates[i] = switches[i].getState() ? ButtonState::SINGLE_PRESSED : ButtonState::NONE;
            } else {
                switchStates[i] = ButtonState::NONE;
            }
        }
    }
    // Accessors for use elsewhere in your program.
    ButtonState getExtendState() const {
        return extendState;
    }
    ButtonState getRetractState() const {
        return retractState;
    }
    ButtonState getSwitchState(int index) const {
        return (index >= 0 && index < MAX_RELAY_PINS) ? switchStates[index] : ButtonState::NONE;
    }
    // Identify whether this is a double-flick of the switch
    ButtonState isSwitchDoubleFlick(int index) {
        previousSwitchActivationTime[index] = thisSwitchActivationTime[index];
        thisSwitchActivationTime[index] = millis();
        if (thisSwitchActivationTime[index] - previousSwitchActivationTime[index] < 1000) {
            return ButtonState::DOUBLE_PRESSED;
        } else {
            return ButtonState::SINGLE_PRESSED;
        }
    }

};
} // namespace ActuatorsController
