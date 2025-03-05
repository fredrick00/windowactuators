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
        totalActuators = numPins / 2; // Calculate total actuators based on numPins
        for (int i = 0; i < numPins; i++) {
            relayPins[i] = pins[i];
        }
        initializeRelays();
    }

int getTotalActuators() const {
        return totalActuators;
}

void controlSingleActuator(int actuatorIndex, bool isExtend) {
    if (relayStates[actuatorIndex].isActive) {
        // The actuator is active, so pause it first
        pauseSingleActuator(actuatorIndex);
    } else {
        // Activate the actuator for the desired action (extend or retract)
        activate(actuatorIndex, isExtend);
    }
}

void pauseSingleActuator(int actuatorIndex) {
    unsigned long currentTime = millis();

    if (relayStates[actuatorIndex].isActive) {
        Serial.print("Pausing Actuator: ");
        Serial.println(actuatorIndex);
        unsigned long elapsed = currentTime - relayStates[actuatorIndex].startTime;
        relayStates[actuatorIndex].remainingTime = max(relayStates[actuatorIndex].remainingTime - elapsed, 0UL);
        if (relayStates[actuatorIndex].isRetracting) {  // Rectify remainingTime from retracting to extending
            relayStates[actuatorIndex].remainingTime = relayStates[actuatorIndex].duration - relayStates[actuatorIndex].remainingTime;
        }

        int pinIndex = relayStates[actuatorIndex].isExtending ? actuatorIndex : actuatorIndex + getTotalActuators();
        digitalWrite(relayPins[pinIndex], HIGH);  // Deactivate the relay
    }

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
                for (int i = 0; i < getTotalActuators(); i++) {
                    activate(i, true);
                }
            }
        } else {
            if (!areAnyExtending()) { // Ensure not currently extending
               if (pausedRemainingTime > 0) {
                pausedRemainingTime = totalDuration - pausedRemainingTime; // rectify from extend to retract
              }
                for (int i = 0; i < getTotalActuators(); i++) {
                    activate(i, false);
                }
            }
        }
      }
    }

void activate(int actuatorIndex, bool isExtend) {
    Serial.print("Activating actuator. (Index/Action): ");
    Serial.print(actuatorIndex);
    Serial.print("/");
    Serial.println(isExtend);

    int pinIndex = isExtend ? actuatorIndex : actuatorIndex + getTotalActuators();

    if (!relayStates[actuatorIndex].isActive) {
        digitalWrite(relayPins[pinIndex], LOW);  // Activate the relay
        relayStates[actuatorIndex].isActive = true;
        relayStates[actuatorIndex].isExtending = isExtend;
        relayStates[actuatorIndex].isRetracting = !isExtend;
        relayStates[actuatorIndex].startTime = millis();
        relayStates[actuatorIndex].remainingTime = pausedRemainingTime ? pausedRemainingTime : totalDuration;
    }
}


    void pauseAll() {
        pausedRemainingTime = 0;
        for (int i = 0; i < numPins; i++) {
            digitalWrite(relayPins[i], HIGH);
            if (relayStates[i].remainingTime) {

              pausedRemainingTime = relayStates[i].remainingTime - (currentMillis - relayStates[i].startTime);
              if (relayStates[i].isRetracting) {
                 pausedRemainingTime = totalDuration - pausedRemainingTime;  // rectify from retract to extend
              }
              relayStates[i].remainingTime = 0;
            }
            relayStates[i].isActive = false;
            relayStates[i].isExtending = false;
            relayStates[i].isRetracting = false;

        }
    }

    bool anyActive() const {
        for (int i = 0; i < numPins; i++) {
            if (relayStates[i].isActive) {
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
    unsigned long currentMillis = millis();

    for (int i = 0; i < getTotalActuators(); i++) {
        if (relayStates[i].isActive) {
            // Calculate elapsed time
            unsigned long elapsedTime = currentMillis - relayStates[i].startTime;


            // Check if the relay should be turned off
            if (elapsedTime >= relayStates[i].remainingTime) {
                int pinIndex = relayStates[i].isExtending ? i : i + getTotalActuators();

            digitalWrite(relayPins[pinIndex], HIGH);  // Stop the actuator
            relayStates[i].isActive = false;
            relayStates[i].isExtending = false;
            relayStates[i].isRetracting = false;

            Serial.print("Actuator ");
            Serial.print(i);
            Serial.println(" stopped.");
          }
        }
    }
}


private:
    int numPins;
    bool activateRelay;
    long pausedRemainingTime = 0;
    int totalActuators; // Encapsulated as a private member

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
          Serial.print (currentState);
          Serial.println (lastState);
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

// Define a class to handle switch interactions
class Switch {
public:
    Switch() : pin(-1), state(false), lastState(false), lastDebounceTime(0), debounceDelay(50) {} 

    Switch(int pin) : pin(pin), state(false), lastState(false), lastDebounceTime(0), debounceDelay(50) {
        pinMode(pin, INPUT_PULLUP);
    }

    // Copy constructor
    Switch(const Switch& other) : pin(other.pin), state(other.state), lastState(other.lastState),
                                  lastDebounceTime(other.lastDebounceTime), debounceDelay(other.debounceDelay) {}

    // Copy assignment operator
    Switch& operator=(const Switch& other) {
        if (this != &other) {
            pin = other.pin;
            state = other.state;
            lastState = other.lastState;
            lastDebounceTime = other.lastDebounceTime;
            // debounceDelay doesn't change after construction, no need to copy it
        }
        return *this;
    }

    bool isPressed() {
        return digitalRead(pin) == LOW;
    }

    bool hasStateChanged() {
        bool currentState = isPressed();
        if (currentState != lastState) {
            lastDebounceTime = millis(); 
        }

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (currentState != state) {
                state = currentState;
                lastState = currentState;
                return true;
            }
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




// Version of this software
const String KitchenScriptVersion = "KitchenWindows v1.0";

// Pin setup and object instantiation
const int extendLedPin = 11; // 
const int retractLedPin = 10; // 
MegaLEDControl leds(extendLedPin, retractLedPin); // Create an instance of MegaLEDControl

const int switchPins[] = {8, 9, 7, 6, 5, 4, 3, 2}; // Define the pins for the 4 momentary switches
const int switchCount = sizeof(switchPins) / sizeof(switchPins[0]);
bool switchStates[] = {false, false, false, false}; // Store the state of each switch
// Declare an array of Switch objects
Switch switches[switchCount]; // Declare the array first

const int extendButtonPin = 12; // Example pin number for the extend button
const int retractButtonPin = 13; // Example pin number for the retract button
MegaButton extendButton(extendButtonPin);  // Creating an instance of MegaButton for extend
MegaButton retractButton(retractButtonPin);  // Creating an instance of MegaButton for retract

const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // actuator relay pinout with extend being first half and retract second half
MegaRelayControl relays(relayPins, sizeof(relayPins) / sizeof(relayPins[0]));  // Creating an instance of MegaRelayControl

// Initialize the MegaActuatorController instance
MegaActuatorController actuatorController(relays, leds);


void setup() {
    // Setup code here, if needed
    Serial.begin(115200);  // Start serial communication at 9600 baud
    Serial2.begin(115200);   // Serial communication with ESP-32
   // Serial2 uses RX (Pin 17) and TX (Pin 16) on Arduino Mega 2560
    relays.initializeRelays(); // Initialize all relays to off
    // create a clear beginning of program execution
        // Initialize switch pins
    // Initialize each switch with its corresponding pin
    for (int i = 0; i < switchCount; i++) {
      switches[i] = Switch(switchPins[i]);
    }
 //   switches[1] = Switch(9);
 //   switches[2] = Switch(7);
//    switches[3] = Switch(6);  
    Serial.print ("\n\n/**\n/**\n/**  Version: ");
    Serial.println (KitchenScriptVersion);
    Serial.println ("/**  Script restarted on Mega board.\n/**\n/**\n");
}

void loop() {
    int simulatedHour = (millis() / 200) % 24;  // Simulate time for demonstration





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
for (int i = 0; i < switchCount; ++i) {
    int pin = switchPins[i];  // Get the pin associated with the current switch
    bool currentState = switches[i].getState();  // Get the current state of the switch
    
    if (switches[i].hasStateChanged()) {
        Serial.print("Switch on pin ");
        Serial.print(pin);
        Serial.print(" (index ");
        Serial.print(i);
        Serial.print(") state changed to ");
        Serial.println(currentState ? "ON" : "OFF");

        // Example: Control actuators based on switch state
        if (currentState) {
            // Activate the corresponding actuator
            Serial.print("Activating actuator for switch on pin ");
            Serial.println(pin);
            relays.controlSingleActuator(i, true); // Assume true for extend
        } else {
            // Pause or deactivate the corresponding actuator
            Serial.print("Pausing actuator for switch on pin ");
            Serial.println(pin);
            relays.pauseSingleActuator(i);
        }
    }
 /*   
    // Print the current state of the switch regardless of state change
    Serial.print("Switch on pin ");
    Serial.print(pin);
    Serial.print(" (index ");
    Serial.print(i);
    Serial.print(") current state: ");
    Serial.println(currentState ? "ON" : "OFF");
    */
    
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
