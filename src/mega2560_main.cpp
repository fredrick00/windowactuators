#ifdef __AVR_ATmega2560__
#include <Arduino.h>
#include "ActuatorReporter.h"
#include "Debounced.h"
#include "megatypes.h"
#include "inputmapping.h"
#include "MegaRelayControl.h"
#include "MegaButton.h"
#include "MegaLEDControl.h"
#include "MegaCommand.h"
#include "MegaActuatorController.h"
#include "MegaInputManager.h"
//#include "MegaSwitch.h"

using namespace ActuatorsController;


// Pin setup and object instantiation
const int extendLedPin = 11; // 
const int retractLedPin = 10; // 
MegaLEDControl leds(extendLedPin, retractLedPin); // Create an instance of MegaLEDControl


//const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // actuator relay pinout with extend being first half and retract second half
inputMappings::MegaRelayControl relays;  // Creating an instance of MegaRelayControl

// Initialize the MegaActuatorController instance
MegaActuatorController actuatorController(relays, leds);

MegaInputManager inputManager;  // Create an instance of MegaInputManager
ActuatorReporter* statusReporter = nullptr;


// Version of this software
const String KitchenScriptVersion = "KitchenWindows V1.14";

void setup() {
    // Setup code here, if needed
    Serial.begin(115200);  // Start serial communication at 115200 baud
    Serial2.begin(115200);   // Serial communication with ESP-32
   // Serial2 uses RX (Pin 17) and TX (Pin 16) on Arduino Mega 2560
    relays.initializeRelays(); // Initialize all relays to off
    statusReporter = new ActuatorReporter(relays);  // Create an instance of ActuatorReporter

    Serial.print ("\n\n/**\n/**\n/**  Version: ");
    Serial.println (KitchenScriptVersion);
    Serial.println ("/**  Script restarted on Mega board.\n/**\n/**\n");
}

void loop() {
    mySwitch.update();
    //inputManager.updateInputs();
    if (mySwitch.isPressed() && mySwitch.stateChanged()) {
        Serial.println("Switch pressed");
        mySwitch.acknowledgeState();
    }
    ButtonState extendButtonState = inputManager.extendButton.getButtonState();
    ButtonState retractButtonState = inputManager.retractButton.getButtonState();
    if (extendButtonState == ButtonState::DOUBLE_PRESSED) {
        Serial.println("\nDouble-press detected on extend button");
        relays.forceOperation(true); // true for extend
    } else if (extendButtonState == ButtonState::SINGLE_PRESSED) {
        Serial.println("\nState changed to extend");
        if (relays.anyActive()) {
            relays.pauseAll();
            leds.setFullBrightness(false, false);
        } else {
            // extend all of the relays
            relays.controlRelays(true);  // Extend relays
            leds.setFullBrightness(true, true);
        }
    } else if (retractButtonState == ButtonState::DOUBLE_PRESSED) {
        Serial.println("\nDouble-press detected on retract button");
        relays.forceOperation(false);  // false for retract
    } else if (retractButtonState == ButtonState::SINGLE_PRESSED) {
      Serial.println("\nState changed to retract");
      if (relays.anyActive()) {
        relays.pauseAll();
        leds.setFullBrightness(false, false);
      } else {
 //       Serial.println("State changed to retract");
        relays.controlRelays(false);  // Retract relays
      }
    }
    if (!relays.anyActive()) {
//        Serial.println("No relays active");
 //       leds.checkNightMode(simulatedHour);
        leds.updateBlink(millis());
    } else {
        if (relays.areAnyExtending()) {
//            Serial.println("Some relays are extending");
            leds.setFullBrightness(true, true);
        } else if (relays.areAnyRetracting()) {
 //           Serial.println("Some relays are retracting");
            leds.setFullBrightness(true, false);
        } else {
 //           Serial.println("Active relays are neither extending nor retracting");
        }
    }

    // Check each switch and control the corresponding actuator
for (int i = 0; i < MAX_PINS; ++i) {
    ButtonState currentState = inputManager.getSwitchState(i);  // Get the current state of the switch
    if (inputManager.getSwitchState(i) == ButtonState::SINGLE_PRESSED) {
        // Check if this is a double-flick indicating a FORCE Extend/Retract command.
        if (inputManager.isSwitchDoubleFlick(i) == ButtonState::DOUBLE_PRESSED) {
            Serial.print("Force Extend/Retract command detected on switch on pin ");
            Serial.println(i);
            // extend or retract this only this pin using the force
            relays.forceOperator(i);  // false for retract
        } else if (currentState == ButtonState::SINGLE_PRESSED) {
            // Activate the corresponding actuator
            Serial.print("Activating actuator for switch on pin ");
            Serial.println(i);
            relays.controlSingleActuator(i);
        } else {
            // Pause or deactivate the corresponding actuator
            Serial.print("Pausing actuator for switch on pin ");
            Serial.println(i);
            relays.pauseSingleActuator(i);
        }
    }
}




    // Read commands from Serial1 (from ESP32) and execute them
    if (Serial2.available()) {
        String commandStr = Serial2.readStringUntil('\n');
        Serial.print ("Serial2 available.  String content: ");
        Serial.print (commandStr);
        commandStr.trim(); // Remove any extraneous whitespace or newline characters
        if (commandStr.length() > 0) {
            MegaCommand command(commandStr);
            Serial.print ("Received command: ");
            Serial.println (commandStr);
            actuatorController.executeCommand(command);
        }
    } else {
//      Serial.println ("Error: Serial1 (ESP32) not available.");
//      delay (2000);
    }
    relays.update();  // Update relay states
}


#endif
