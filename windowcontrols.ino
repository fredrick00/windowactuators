#ifdef __AVR_ATmega2560__

#include <Arduino.h>

class MegaRelayControl {
public:
    static const int MAX_PINS = 8; // Define the maximum number of relays you can control
    int relayPins[MAX_PINS]; // Static array to hold relay pins
    struct RelayState {
        bool isActive;
        bool isExtending;
        bool isRetracting;
        unsigned long startTime;
        unsigned long remainingTime;
        unsigned long duration;  

    } relayStates[MAX_PINS]; // Static array for relay states

    MegaRelayControl
  (const int* pins, int numPins) : numPins(numPins) {
        for (int i = 0; i < numPins; i++) {
            relayPins[i] = pins[i];
        }
        initializeRelays();
    }

    void controlSingleActuator(int actuatorIndex, bool isExtend) {
        int pinIdx = isExtend ? actuatorIndex * 2 : actuatorIndex * 2 + 1;
        if (relayStates[actuatorIndex].isActive && ((isExtend && !relayStates[actuatorIndex].isExtending) || (!isExtend && !relayStates[actuatorIndex].isRetracting))) {
            // The actuator is active but in the opposite state, so pause it first
            pauseSingleActuator(actuatorIndex);
        }
        activate(actuatorIndex, isExtend); // Call activate with the index and action
    }

    void pauseSingleActuator(int actuatorIndex) {
        unsigned long currentTime = millis();
        if (relayStates[actuatorIndex].isActive) {
            unsigned long elapsed = currentTime - relayStates[actuatorIndex].startTime;
            relayStates[actuatorIndex].remainingTime = max(relayStates[actuatorIndex].remainingTime - elapsed, 0UL);
            if (relayStates[actuatorIndex].isRetracting) { // rectify remainingTime from retracting to extending
                relayStates[actuatorIndex].remainingTime = relayStates[actuatorIndex].duration - relayStates[actuatorIndex].remainingTime;
            }
        }
        digitalWrite(relayPins[actuatorIndex * 2], HIGH);
        digitalWrite(relayPins[actuatorIndex * 2 + 1], HIGH);
        relayStates[actuatorIndex].isActive = false;
        relayStates[actuatorIndex].isExtending = false;
        relayStates[actuatorIndex].isRetracting = false;
    }


    void initializeRelays() {
      pausedRemainingTime = 0;
        for (int i = 0; i < numPins; i++) {
            pinMode(relayPins[i], OUTPUT);
            digitalWrite(relayPins[i], HIGH); // Assuming HIGH means relay off
            relayStates[i].isActive = false;
            relayStates[i].isExtending = false;
            relayStates[i].isRetracting = false;
            relayStates[i].startTime = 0;
            relayStates[i].remainingTime = 0;
        }
    }

    void controlRelays(bool isExtend) {
      if (anyActive()) {
        pauseAll ();
      } else {
        if (isExtend) { 
            if (!areAnyRetracting()) { // Ensure not currently retracting
                for (int i = 0; i < numPins / 2; i++) {
                    activate(i, true);
                }
            }
        } else {
            if (!areAnyExtending()) { // Ensure not currently extending
              if (pausedRemainingTime > 0) {
                pausedRemainingTime = totalDuration - pausedRemainingTime; // rectify from extend to retract
              }
                for (int i = numPins / 2; i < numPins; i++) {
                    activate(i, false);
                }
            }
        }
      }
    }

    void activate(int index, bool action) {
        Serial.print ("Activating actuator: ");
        Serial.println (index);
        if (index < numPins && !relayStates[index].isActive) {
            digitalWrite(relayPins[index], LOW); // Activate the relay
            relayStates[index].isActive = true;
            relayStates[index].isExtending = action;
            relayStates[index].isRetracting = !action;
            relayStates[index].startTime = millis();
            relayStates[index].remainingTime = pausedRemainingTime ? pausedRemainingTime : totalDuration;
        }
    }

    void pauseAll() {
        pausedRemainingTime = 0;
        for (int i = 0; i < numPins; i++) {
            digitalWrite(relayPins[i], HIGH);
            if (relayStates[i].remainingTime) {

              pausedRemainingTime = relayStates[i].remainingTime - (currentMillis - relayStates[i].startTime);
Serial.println (pausedRemainingTime);
              if (relayStates[i].isRetracting) {
                 pausedRemainingTime = totalDuration - pausedRemainingTime;  // rectify from retract to extend
              }
              relayStates[i].remainingTime = 0;
                            Serial.print ("setting remaining time to: ");
              Serial.println (pausedRemainingTime);
            }
            relayStates[i].isActive = false;
            relayStates[i].isExtending = false;
            relayStates[i].isRetracting = false;

        }
    }

    bool anyActive() const {
        for (int i = 0; i < numPins; i++) {
            if (relayStates[i].isActive) {
              Serial.print ("relaystate active: ");
              Serial.println (i);
                return true;
            }
        }
        return false;
    }

    bool areAnyExtending() const {
        for (int i = 0; i < numPins; i++) {
            if (relayStates[i].isExtending) {
                return true;
            }
        }
        return false;
    }

    bool areAnyRetracting() const {
        for (int i = numPins / 2; i < numPins; i++) {
            if (relayStates[i].isRetracting) {
                return true;
            }
        }
        return false;
    }

    void update() {
        currentMillis = millis();
        for (int i = 0; i < numPins; i++) {
            if (relayStates[i].isActive && (currentMillis - relayStates[i].startTime >= relayStates[i].remainingTime)) {
                digitalWrite(relayPins[i], HIGH);
                relayStates[i].isActive = false;
                relayStates[i].isExtending = false;
                relayStates[i].isRetracting = false;
            }
        }
    }

private:
    int numPins;
    bool activateRelay;
    long pausedRemainingTime = 0;
    unsigned long currentMillis = millis();
    const unsigned long totalDuration = 5000; // Total duration in milliseconds
};


// MegaButton class to handle button state changes
class MegaButton {
public:
    MegaButton(int pin) : pin(pin), lastState(HIGH) {
        pinMode(pin, INPUT_PULLUP);
        lastState = digitalRead(pin);  // Initialize lastState with the current state of the button

    }

    bool stateChanged() {
        int currentState = digitalRead(pin);
        if (currentState != lastState) {
            lastState = currentState;
            
            return true;
        }
        return false;
    }

private:
    int pin;
    int lastState;
};

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

class MegaCommand {
public:
  MegaCommand(const String& rawCommand) {
    parseCommand(rawCommand);
  }

  const String& getAction() const {
    return action;
  }

  int getActuator() const {
    return actuator;
  }

private:
  String action;
  int actuator;

  void parseCommand(const String& rawCommand) {
    int spaceIndex = rawCommand.indexOf(' ');
    if (spaceIndex > 0) {
      action = rawCommand.substring(0, spaceIndex);
      String actuatorStr = rawCommand.substring(spaceIndex + 1);
      if (actuatorStr == "ALL") {
        actuator = 0;
      } else {
        actuator = actuatorStr.toInt();
      }
    }
  }
};

class MegaActuatorController {
public:
  MegaActuatorController(MegaRelayControl
& relayControl, MegaLEDControl& ledControl)
    : relays(relayControl), leds(ledControl) {}

  void executeCommand(const MegaCommand& command) {
    if (command.getAction() == "EXTEND") {
      if (command.getActuator() == 0) {
        relays.controlRelays(true);
      } else {
        relays.controlSingleActuator(command.getActuator() - 1, true);
      }
      leds.setFullBrightness(true, true);
    } else if (command.getAction() == "RETRACT") {
      if (command.getActuator() == 0) {
        relays.controlRelays(false);
      } else {
        relays.controlSingleActuator(command.getActuator() - 1, false);
      }
      leds.setFullBrightness(true, false);
    }
  }

private:
  MegaRelayControl
& relays;
  MegaLEDControl& leds;
};


// Pin setup and object instantiation
const int extendLedPin = 13; // 
const int retractLedPin = 10; // 
MegaLEDControl leds(extendLedPin, retractLedPin); // Create an instance of MegaLEDControl

const int extendButtonPin = 12; // Example pin number for the extend button
const int retractButtonPin = 11; // Example pin number for the retract button
MegaButton extendButton(extendButtonPin);  // Creating an instance of MegaButton for extend
MegaButton retractButton(retractButtonPin);  // Creating an instance of MegaButton for retract

const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // actuator relay pinout with extend being first half and retract second half
MegaRelayControl relays(relayPins, sizeof(relayPins) / sizeof(relayPins[0]));  // Creating an instance of MegaRelayControl

// Initialize the MegaActuatorController instance
MegaActuatorController actuatorController(relays, leds);


void setup() {
    // Setup code here, if needed
    Serial.begin(9600);  // Start serial communication at 9600 baud
    Serial1.begin(115200);   // Serial communication with ESP-32
    relays.initializeRelays(); // Initialize all relays to off

}

void loop() {
    int simulatedHour = (millis() / 200) % 24;  // Simulate time for demonstration
        Serial.println("\n checking state change:");

    if (extendButton.stateChanged()) {
        Serial.println("\nState changed to extend");
      if (relays.anyActive()) {
        relays.pauseAll();
        leds.setFullBrightness(false, false);
      } else {
        relays.controlRelays(true);  // Extend relays
      }
    } else if (retractButton.stateChanged()) {
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
        Serial.println("No relays active");
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

    relays.update();  // Update relay states

    // Read commands from Serial1 (from ESP32) and execute them
    if (Serial1.available()) {
        String commandStr = Serial1.readStringUntil('\n');
        commandStr.trim(); // Remove any extraneous whitespace or newline characters
        if (commandStr.length() > 0) {
            MegaCommand command(commandStr);
            Serial.print ("executing command: ");
            Serial.println (commandStr);
            actuatorController.executeCommand(command);
        }
    }
}

#endif
