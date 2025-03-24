//
// Created by fredr on 3/13/2025.
//
#pragma once

#include <Arduino.h>
#include "inputmapping.h"
#include "MegaRelayControl.h"

using namespace ActuatorsController;

class MegaRelayControl {
public:
    Mode stateReport;

    struct RelayState {
        bool stateHasChanged;
        String actuatorName;
        bool isActive;
        Mode relayState;
        unsigned long startTime;
        unsigned long actuatorPosition;
        unsigned long maxDuration;

    };
    // Static array for relay states
    RelayState relayStates[MAX_RELAY_PINS];

    MegaRelayControl
  () {
        initializeRelays();
    }

bool isForceMode() const {
    return forcedActive;
}


void forceOperator (int actuatorIndex) {
    Serial.print("FORCED OPERATION: ");
    Serial.println(actuatorIndex);
    if (!relayStates[actuatorIndex].isActive) {
        activate(actuatorIndex); //
    }
}
// -------END Force Function----------- // *****


// Initiates a forced operation for all actuators.
void forceOperation(bool isExtend) {
    Serial.print("FORCED OPERATION (all actuators): ");
    Serial.println(isExtend ? "EXTENDING" : "RETRACTING");
    forcedActive = true;
    forcedStartTime = millis();
    for (int i = 0; i < MAX_RELAY_PINS; i++) {
        if (!relayStates[i].isActive && (inputMappings[i].mode == Mode::EXTENDING) == isExtend) {
            forceOperator(i);
        }
    }
}
// -------END Force Function----------- // *****


void controlSingleActuator(int actuatorIndex) {
    if (relayStates[actuatorIndex].isActive && !isForceMode()) {
        // The actuator is active, so pause it first
        pauseSingleActuator(actuatorIndex);
    } else {
        // Activate the actuator for the desired action (extend or retract)
        activate(actuatorIndex);
    }
}


    void initializeRelays() {
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            pinMode(inputMappings[i].actuatorPin, OUTPUT);
            digitalWrite(inputMappings[i].actuatorPin, HIGH); // Assuming HIGH means relay off
            stateChanged = true; // start by generating a report
            relayStates[i].stateHasChanged = true; // start with a generated report
            relayStates[i].isActive = false;
            relayStates[i].actuatorName = inputMappings[i].actuatorName;
            relayStates[i].startTime = 0;
            relayStates[i].actuatorPosition = 0;
            relayStates[i].maxDuration = maxDuration;
            relayStates[i].relayState = Mode::PAUSED;
        }
    }

    void controlRelays(bool isExtend) {

      if (anyActive() && !forcedActive) {
        pauseAll ();
      } else {
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            // activate relays for the action indicated by isExtend
            if ((isExtend && inputMappings[i].mode == Mode::EXTENDING) ||
                (!isExtend && inputMappings[i].mode == Mode::RETRACTING)) {
                activate(i);
            }

        }
      }
    }

void activate(int actuatorIndex) {
    Serial.print("Activating actuator. (Pin/Action): ");
    Serial.print(inputMappings[actuatorIndex].actuatorPin);
    Serial.print("/");
    Serial.println((inputMappings[actuatorIndex].mode == Mode::EXTENDING) ? "EXTENDING" : "RETRACTING");

       // if this actuator is not active.
        if (!relayStates[actuatorIndex].isActive) {
            relayStates[actuatorIndex].isActive = true;
            digitalWrite(inputMappings[actuatorIndex].actuatorPin, LOW);  // Activate the relay
            stateChanged = true; // generate a report upon launch.
            // call the MegaRelayControl setRelayChangedState function to set stateHasChanged to true.
            relayStates[actuatorIndex].stateHasChanged = true;
            const char * thisActuatorName = inputMappings[actuatorIndex].actuatorName;
            unsigned long thisActuatorPosition = relayStates[actuatorIndex].actuatorPosition;
            for (int i = 0; i < MAX_RELAY_PINS; i++) {
                if (inputMappings[i].actuatorName == thisActuatorName) {
                    relayStates[i].relayState = inputMappings[actuatorIndex].mode;
                    relayStates[i].actuatorPosition = thisActuatorPosition;
                    relayStates[i].startTime = millis();
                }
            }
        }
        Serial.print("Actuator Position: ");
        Serial.println (relayStates[actuatorIndex].actuatorPosition);
    }

void pauseSingleActuator(int actuatorIndex) {
    unsigned long currentTime = millis();

    if (relayStates[actuatorIndex].isActive) {

        Serial.print("Pausing Actuator on pin: ");
        Serial.print(inputMappings[actuatorIndex].actuatorPin);
        // if this is a retracting actuator
        if (inputMappings[actuatorIndex].mode == Mode::RETRACTING)
        {  // when retracting we subtract from the duration.
            if (relayStates[actuatorIndex].actuatorPosition < currentTime - relayStates[actuatorIndex].startTime)
            {
                relayStates[actuatorIndex].actuatorPosition = 0UL;
            } else {
                relayStates[actuatorIndex].actuatorPosition = relayStates[actuatorIndex].actuatorPosition - (currentTime - relayStates[actuatorIndex].startTime);
            }
        } else {
            relayStates[actuatorIndex].actuatorPosition = relayStates[actuatorIndex].actuatorPosition + (currentTime - relayStates[actuatorIndex].startTime);
        }
        Serial.print(" @: ");
        Serial.println(relayStates[actuatorIndex].actuatorPosition);

        digitalWrite(inputMappings[actuatorIndex].actuatorPin, HIGH);  // Deactivate the relay
        // save this relays changed state, as well as flagging that a state has changed in any relay.
        relayStates[actuatorIndex].stateHasChanged = true;
        stateChanged = true;
        const char* thisActuatorName = inputMappings[actuatorIndex].actuatorName;
        unsigned long thisActuatorPosition = relayStates[actuatorIndex].actuatorPosition;
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            if (inputMappings[i].actuatorName == thisActuatorName) {
                relayStates[i].isActive = false;
                relayStates[i].relayState = Mode::PAUSED;
                relayStates[i].actuatorPosition = thisActuatorPosition;
            }
        }
    }
}


    void pauseAll() {
        Serial.println ("Pausing all actuators.");
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            pauseSingleActuator(i);
        }
    }

    bool anyActive() const {
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            if (relayStates[i].isActive) {
                return true;
            }
        }
        return false;
    }

    bool areAnyExtending() const {
        for (int i = 0; i < MAX_RELAY_PINS; i++) {
            if (relayStates[i].relayState == Mode::EXTENDING) {
                return true;
            }
        }
        return false;
    }

    bool areAnyRetracting() const {
        for (int i = MAX_RELAY_PINS / 2; i < MAX_RELAY_PINS; i++) {
            if (relayStates[i].relayState == Mode::RETRACTING) {
                return true;
            }
        }
        return false;
    }

// return if there has been a state change to trigger reporting
bool getChangedState() {
    bool thisStateChanged = stateChanged;
    stateChanged = false;
    return thisStateChanged;
}

bool hasRelayChangedState(int relayIndex) const {
  return relayStates[relayIndex].stateHasChanged;
}

void setRelayChangedState(int relayIndex, bool stateHasChanged) {
    relayStates[relayIndex].stateHasChanged = stateHasChanged;
}

void update() {
    unsigned long currentTime = millis();

    if (forcedActive && (currentTime - forcedStartTime >= FORCED_DURATION)) {
        Serial.println("Forced operation expired. Pausing all actuators.");
        forcedActive = false;
        pauseAll();
    }

    for (int i = 0; i < MAX_RELAY_PINS; i++) {
        if (relayStates[i].isActive) {
            // Calculate elapsed time
            unsigned long elapsedTime = currentTime - relayStates[i].startTime;
            // The new duration is the previous duration + for extend or - for retract the elapsed time.
            unsigned long newDuration = 0;
            if (inputMappings[i].mode == Mode::EXTENDING) {
                newDuration = relayStates[i].actuatorPosition + elapsedTime;
            } else if (inputMappings[i].mode == Mode::RETRACTING) { // retracting
                newDuration = (relayStates[i].actuatorPosition < elapsedTime) ? 0UL
                                  : relayStates[i].actuatorPosition - elapsedTime;
            }
            // Check if the relay should be turned off
            // let it keep running if in force mode.
            if (!isForceMode()) {
                // if the newDuration is > 0 or < duration, then we keep going.
                if ((newDuration <= 0 && relayStates[i].relayState == Mode::RETRACTING)
                    || (newDuration > relayStates[i].maxDuration && relayStates[i].relayState == Mode::EXTENDING)) {
                    pauseSingleActuator(i);
                }
            }

          }
        }
    }

private:

    //unsigned long currentMillis = millis();
    // Max duration in milliseconds before duration is capped.
    const unsigned long maxDuration = 8000;

    // Members and method to enable a forced extend/retract mode.
    bool forcedActive = false; // True when a forced operation is active
    unsigned long forcedStartTime = 0; // Timestamp when the forced mode started
    static const unsigned long FORCED_DURATION = 5000UL; // Forced operation lasts 5000 ms
    bool stateChanged = false; // Monitor whether anything has changed state for report generation.
};

