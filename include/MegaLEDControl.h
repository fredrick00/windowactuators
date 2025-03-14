//
// Created by fredr on 3/13/2025.
//
#pragma once
#include <Arduino.h>

// MegaLEDControl class to handle LED operations
class MegaLEDControl {
public:
    MegaLEDControl(int extendLedPin, int retractLedPin)
        : extendLedPin(extendLedPin), retractLedPin(retractLedPin), previousMillis(0), extendLedState(false), retractLedState(false), nightMode(false) {
        pinMode(extendLedPin, OUTPUT);
        pinMode(retractLedPin, OUTPUT);
        previousMillis = millis();
    }

    void updateBlink(unsigned long currentMillis) {
 //       if (nightMode) {
            if ((extendLedState && currentMillis - previousMillis >= 1000) ||
                (!extendLedState && currentMillis - previousMillis >= 3000)) {
                extendLedState = !extendLedState;
                retractLedState = !retractLedState;
                analogWrite(extendLedPin, extendLedState ? 28 : 0);  // Toggle between 50% and 0% brightness
                analogWrite(retractLedPin, retractLedState ? 28 : 0);  // Toggle between 50% and 0% brightness
                previousMillis = currentMillis;
            }
        }
//    }

    void setFullBrightness(bool on, bool isExtend) {

        if (on) {
            analogWrite(extendLedPin, isExtend ? 255 : 0); // Full brightness for extend LED
            analogWrite(retractLedPin, !isExtend ? 255 : 0); // Full brightness for retract LED
                    previousMillis = millis(); // Reset timing when lights are turned on to start the timing for blink/off cycle from here
        } else {
            analogWrite(retractLedPin, 0);
            analogWrite(extendLedPin, 0);
        }
    }

    void checkNightMode(int hour) {
        if (hour >= 18 || hour < 6) {
            nightMode = true;
        } else {
            nightMode = false;
        }
    }

private:
    int extendLedPin, retractLedPin;
    unsigned long previousMillis;
    bool extendLedState, retractLedState;
    bool nightMode;

};
