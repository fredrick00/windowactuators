//
// Created by fredr on 3/14/2025.
//
#pragma once
#include <Arduino.h>




// Define a class to handle switch interactions
class MegaSwitch {
public:
    MegaSwitch() : pin(-1), state(false), lastState(false), lastDebounceTime(0), debounceDelay(50) {}

    MegaSwitch(int pin) : pin(pin), state(false), lastState(false), lastDebounceTime(0), debounceDelay(50) {
        pinMode(pin, INPUT_PULLUP);
    }

    // Copy constructor
    MegaSwitch(const MegaSwitch& other) : pin(other.pin), state(other.state), lastState(other.lastState),
                                  lastDebounceTime(other.lastDebounceTime), debounceDelay(other.debounceDelay) {}

    // Copy assignment operator
    MegaSwitch& operator=(const MegaSwitch& other) {
        if (this != &other) {
            pin = other.pin;
            state = other.state;
            lastState = other.lastState;
            lastDebounceTime = other.lastDebounceTime;
            // debounceDelay doesn't change after construction, no need to copy it
        }
        return *this;
    }

    bool isPressed() const {
        return digitalRead(pin) == LOW;
    }

    bool hasStateChanged() {
        bool currentState = isPressed();
        if (currentState != lastState) {
            lastDebounceTime = millis();
        }

        if ((millis() - lastDebounceTime) > debounceDelay && currentState != state) {
                state = currentState;
                lastState = currentState;
                return true;
        }

        lastState = currentState;
        return false;
    }

    bool getState() const {
        return state;
    }

private:
    int pin;
    bool state;
    bool lastState;
    unsigned long lastDebounceTime;
    const unsigned long debounceDelay;
};

