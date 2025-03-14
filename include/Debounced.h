//
// Created by fredr on 3/13/2025.
//
#pragma once
#include <Arduino.h>

namespace ActuatorsController {

// Class for handling debouncing as we check our inputs
// C++ code (e.g., for Arduino)
class Debounced {
public:
    Debounced() = default; // Default constructor for std::array

    // Constructor:
    // - pin: the digital pin connected to the switch.
    // - debounceInterval: minimum time in milliseconds the input must remain stable before updating the state.
    Debounced(int pin, unsigned long debounceInterval = 50)
    : pin(pin),
      debounceInterval(debounceInterval),
      lastStableState(HIGH),
      lastReading(HIGH),
      lastChangeTime(0),
      lastReportedState(HIGH)
    {
        pinMode(pin, INPUT_PULLUP);
    }

    // initialize() method to use if using default constructor first.
    void initialize(int pin, unsigned long debounceInterval = 50) {
        this->pin = pin;
        this->debounceInterval = debounceInterval;
        lastStableState = HIGH;
        lastReading = HIGH;
        lastChangeTime = 0;
        lastReportedState = HIGH;
        pinMode(pin, INPUT_PULLUP);
    }


    // Call this method in the loop() frequently.
    // It updates the lastStableState only if the input reading has been stable for debounceInterval.
    void update() {
        int currentReading = digitalRead(pin);

        // If the reading has changed, record the time.
        if (currentReading != lastReading) {
            lastChangeTime = millis();
            lastReading = currentReading;
        }

        // If the reading remains stable and the debounce interval has elapsed, update the state.
        if ((millis() - lastChangeTime) >= debounceInterval && currentReading != lastStableState) {
            lastStableState = currentReading;
        }
    }

    // Returns the debounced state of the switch.
    bool isPressed() const {
        // Assuming LOW means button pressed when using INPUT_PULLUP.
        return (lastStableState == LOW);
    }

    // Optionally, you can add a method to detect state changes.
    bool stateChanged() const {
        return (lastStableState != lastReportedState);
    }

    // Call this whenever you've acted upon a state change.
    void acknowledgeState() const {
        lastReportedState = lastStableState;
    }

private:
    int pin;
    unsigned long debounceInterval;
    int lastStableState;     // The debounced stable state (HIGH or LOW)
    int lastReading;         // The last immediate reading from digitalRead()
    unsigned long lastChangeTime;  // The time when the last change was detected
    mutable int lastReportedState = HIGH;  // Store the previously reported state
};
} // namespace ActuatorsController
