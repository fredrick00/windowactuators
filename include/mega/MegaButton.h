//
// Created by fredr on 3/13/2025.
//
#pragma once
#include <Arduino.h>
#include "megatypes.h"

namespace ActuatorsController {
// MegaButton class to handle button state changes
class MegaButton {
public:
    MegaButton(int pin) : pin(pin), lastState(HIGH), lastPressTime(0) {
        pinMode(pin, INPUT_PULLUP);
        lastState = digitalRead(pin);  // Initialize lastState with the current state of the button
    }


    // Basic state change detection (debounced) bool
    bool stateChanged() {
        const int debounceChecks = 5;
        const int debounceDelayMs = 10;
        int persistentState = digitalRead(pin);
        // Check pin multiple times with a delay
        for (int i = 0; i < debounceChecks; i++) {
            delay(debounceDelayMs);
            if (digitalRead(pin) != persistentState) {
                // Not consistent, return false
                return false;
            }
        }
        if (persistentState != lastState) {
            lastState = persistentState;
            return true;
        }
        return false;
    }

    bool isDoublePressed() {
        lastPressTime = currentPressTime;
        currentPressTime = millis();
        if (currentPressTime - lastPressTime < threshold) {
           return true;
        }
        return false;
    }

    // Returns a ButtonEvent indicating the button press nature ButtonEvent
    ButtonState getButtonState() {
        if (stateChanged()) {
            if (isDoublePressed()) {
                return ButtonState::DOUBLE_PRESSED;
            } else {
                return ButtonState::SINGLE_PRESSED;
            }
        } return ButtonState::NONE;
    }
private:
    int pin;
    int lastState;
    static const int threshold = 1000; // Debounce threshold in milliseconds
    unsigned long currentPressTime = millis();;
    unsigned long lastPressTime = currentPressTime;
};
} // namespace ActuatorsController