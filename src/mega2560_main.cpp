#ifdef __AVR_ATmega2560__

#include <Arduino.h>
#include "megatypes.h"


class MegaRelayControl {
public:
    static const int MAX_PINS = 8; // Define the maximum number of relays you can control
    int relayPins[MAX_PINS]; // Static array to hold relay pins

    struct RelayState {
        bool isActive;
        bool isExtending;
        bool isRetracting;
        unsigned long startTime;
        unsigned long actuatorPosition;
        unsigned long maxDuration;

    } relayStates[MAX_PINS]; // Static array for relay states

    MegaRelayControl
  (const int* pins, int numPins) : numPins(numPins) {
        //  totalActuators = numPins / 2; // Calculate total actuators based on numPins
        for (int i = 0; i < numPins; i++) {
            relayPins[i] = pins[i];
        }
        initializeRelays();
    }

static int getTotalActuators() {
        return totalActuators;
}

void forceOperator (int pin) {
    Serial.print("FORCED OPERATION: ");
    Serial.println(pin);
    if (!relayStates[pin].isActive) {
        activate(pin, 1); // Always send true for individual pins.
    }
    forcedActiveSwitch = true;
    forcedIsExtend = 1;
    forcedStartTime = millis();
}
// -------END Force Function----------- // *****


// Initiates a forced operation for all actuators.
void forceOperation(bool isExtend) {
    Serial.print("FORCED OPERATION: ");
    Serial.println(isExtend ? "EXTEND" : "RETRACT");
    for (int i = 0; i < getTotalActuators(); i++) {
        if (!relayStates[i].isActive) {
            activate(i, isExtend);
        }
    }
    forcedActive = true;
    forcedIsExtend = isExtend;
    forcedStartTime = millis();
}
// -------END Force Function----------- // *****


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
        //unsigned long currentExtendedDuration =
    //relayStates[actuatorIndex].isExtending
        //? (relayStates[actuatorIndex].durationExtended + (currentTime - relayStates[actuatorIndex].startTime))
        //: (relayStates[actuatorIndex].durationExtended < (currentTime - relayStates[actuatorIndex].startTime)
              //? 0UL
              //: (relayStates[actuatorIndex].durationExtended - (currentTime - relayStates[actuatorIndex].startTime)));

       Serial.print("Pausing Actuator: ");
        Serial.print(actuatorIndex);
        if (relayStates[actuatorIndex].isRetracting) {  // when retracting we subtract from the duration.
            if (relayStates[actuatorIndex].actuatorPosition < currentTime - relayStates[actuatorIndex].startTime) {
                relayStates[actuatorIndex].actuatorPosition = 0UL;
            } else {
                relayStates[actuatorIndex].actuatorPosition = relayStates[actuatorIndex].actuatorPosition - (currentTime - relayStates[actuatorIndex].startTime);
            }
        } else {
            relayStates[actuatorIndex].actuatorPosition = relayStates[actuatorIndex].actuatorPosition + (currentTime - relayStates[actuatorIndex].startTime);
        }
        Serial.print(" @: ");
        Serial.println(relayStates[actuatorIndex].actuatorPosition);

        int pinIndex = relayStates[actuatorIndex].isExtending ? actuatorIndex : actuatorIndex + getTotalActuators();
        digitalWrite(relayPins[pinIndex], HIGH);  // Deactivate the relay
    }

    relayStates[actuatorIndex].isActive = false;
    relayStates[actuatorIndex].isExtending = false;
    relayStates[actuatorIndex].isRetracting = false;
}


    void initializeRelays() {
        for (int i = 0; i < numPins; i++) {
            pinMode(relayPins[i], OUTPUT);
            digitalWrite(relayPins[i], HIGH); // Assuming HIGH means relay off
            relayStates[i].isActive = false;
            relayStates[i].isExtending = false;
            relayStates[i].isRetracting = false;
            relayStates[i].startTime = 0;
            relayStates[i].actuatorPosition = 0;
            relayStates[i].maxDuration = maxDuration;
        }
    }

    void controlRelays(bool isExtend) {

      if (anyActive() && !forcedActive) {
        pauseAll ();
      } else {
        if (isExtend) { 
                for (int i = 0; i < getTotalActuators(); i++) {
                    activate(i, true);
                }
        } else {
                for (int i = 0; i < getTotalActuators(); i++) {
                    // adjust totalDuration to match the rectified remainingTime[i] if there is a remaining time.
                    //totalDuration = remainingTimes[i] ? remainingTimes[i] : totalDuration;
                    activate(i, false);
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
       // if this actuator is not active.
    if (!relayStates[actuatorIndex].isActive) {
        digitalWrite(relayPins[pinIndex], LOW);  // Activate the relay
        relayStates[actuatorIndex].isActive = true;
        relayStates[actuatorIndex].isExtending = isExtend;
        relayStates[actuatorIndex].isRetracting = !isExtend;
        relayStates[actuatorIndex].startTime = millis();
        Serial.print("Actuator Position: ");
        Serial.println (relayStates[actuatorIndex].actuatorPosition);
    }
}


    void pauseAll() {
        Serial.println ("Pausing all actuators.");
        // We don't pause if we're forcing the actuators
        if (forcedActive) {
            return;
        }
        for (int i = 0; i < getTotalActuators(); i++) {
            pauseSingleActuator(i);
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
    unsigned long currentTime = millis();
    if (forcedActive && (currentTime - forcedStartTime >= FORCED_DURATION)) {
        Serial.println("Forced operation expired. Pausing all actuators.");
        forcedActive = false;
        pauseAll();
    }

    for (int i = 0; i < getTotalActuators(); i++) {
        if (relayStates[i].isActive) {
            // Calculate elapsed time
            unsigned long elapsedTime = currentTime - relayStates[i].startTime;
            // The new duration is the previous duration + for extend or - for retract the elapsed time.
            unsigned long newDuration = 0;
            if (relayStates[i].isExtending) {
                // ensure newDuration does not start at 0.
                delay(1);
                newDuration = relayStates[i].actuatorPosition + elapsedTime;
            } else { // retracting
                newDuration = (relayStates[i].actuatorPosition < elapsedTime) ? 0UL
                                  : relayStates[i].actuatorPosition - elapsedTime;
            }
            // Check if the relay should be turned off
            // if the newDuration is > 0 or < duration, then we keep going.
            if ((newDuration <= 0 && relayStates[i].isRetracting )|| (newDuration > relayStates[i].maxDuration && relayStates[i].isExtending)) {
                    pauseSingleActuator(i);
                    //digitalWrite(relayPins[pinIndex], HIGH);  // Stop the actuator
                    //relayStates[i].isActive = false;
                    //relayStates[i].isExtending = false;
                    //relayStates[i].isRetracting = false;
           }

          }
        }
    }

private:
    int numPins;
    bool activateRelay;
    static const int totalActuators = MAX_PINS / 2;
    unsigned long remainingTimes[totalActuators];

    //unsigned long currentMillis = millis();
    // Max duration in milliseconds before duration is capped.
    const unsigned long maxDuration = 8000;

    // Members and method to enable a forced extend/retract mode.
    bool forcedActive = false; // True when a forced operation is active
    bool forcedActiveSwitch = false;
    bool forcedIsExtend = false; // True for extend, false for retract during forced operation
    unsigned long forcedStartTime = 0; // Timestamp when the forced mode started
    static const unsigned long FORCED_DURATION = 5000UL; // Forced operation lasts 5000 ms
};




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
    unsigned long currentPressTime;
    unsigned long lastPressTime;
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


//-------------------------------------------------------------------- //
// MegaInputManager class: Reads two overall buttons and an array // of switch inputs (assumed here to be four physical switches).
class MegaInputManager {


private:
    // Switches for individual control.
    static const int numSwitchPins = 8;
    // Hardware buttons for overall extend/retract actions.
    static const int extendButtonPin = 12; // Example pin for extend
    static const int retractButtonPin = 13; // Example pin for retract
    // Here we assume there are four physical switches.
    const int switchPins[numSwitchPins] = {8, 9, 7, 6, 5, 4, 3, 2};
    unsigned long thisSwitchActivationTime[numSwitchPins];
    unsigned long previousSwitchActivationTime[numSwitchPins];
    SwitchMode switchMode[numSwitchPins];
    int switch2Actuator[numSwitchPins];




    public:
    // Use the same enum for button and switch events.
    MegaButton extendButton;
    MegaButton retractButton;
    Switch switches[numSwitchPins]; // Use the same type as the events produced by buttons.
    ButtonState switchStates[numSwitchPins];
    // State variables for the extend and retract buttons.
    ButtonState extendState;
    ButtonState retractState;

    // Constructor: Create the two buttons and initialize the switch array.
    MegaInputManager() : extendButton(extendButtonPin), retractButton(retractButtonPin), extendState(ButtonState::NONE), retractState(ButtonState::NONE) {
        // Initialize each physical switch with its corresponding pin.
        for (int i = 0; i < getNumSwitchPins(); i++) {
            switches[i] = Switch(switchPins[i]);
            switchStates[i] = ButtonState::NONE;
            if (i & 1) { // Will evaluate true for odd numbers
                // We will assume that numSwitchPins == 2 * number of actuators.
                switch2Actuator[i] = i / 2; // For retract, actuator number is i - totalActuators.
                switchMode[i] = SwitchMode::RETRACT; // Set the switch mode to extend/retract
            } else {
                switch2Actuator[i] = i / 2;
                switchMode[i] = SwitchMode::EXTEND;
            }
            // timestamp for current and previous switch activation to detect FORCE extend/retract request.
            // for loop to cycle through switches and initiate thisSwitchActivationTime and previousSwitchActivationTime arrays to numSwitchPins in size.
            thisSwitchActivationTime[i] = 0;
            previousSwitchActivationTime[i] = 0;
        }

    }

    // get function
    static const int getNumSwitchPins() {
        return numSwitchPins;
    }
    // Call this each loop to measure the current inputs.
    void updateInputs() {
        // Update the overall buttons.
        extendState = extendButton.getButtonState();
        retractState = retractButton.getButtonState();
        // Update each switch.
        for (int i = 0; i < getNumSwitchPins(); i++) {
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
        return (index >= 0 && index < getNumSwitchPins()) ? switchStates[index] : ButtonState::NONE;
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

    SwitchMode getSwitchMode(int index) {
        return switchMode[index];
    }

    int getSwitch2Actuator(int index) {
        return switch2Actuator[index];
    }
};





// Version of this software
const String KitchenScriptVersion = "KitchenWindows V1.13";

// Pin setup and object instantiation
const int extendLedPin = 11; // 
const int retractLedPin = 10; // 
MegaLEDControl leds(extendLedPin, retractLedPin); // Create an instance of MegaLEDControl


const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // actuator relay pinout with extend being first half and retract second half
MegaRelayControl relays(relayPins, sizeof(relayPins) / sizeof(relayPins[0]));  // Creating an instance of MegaRelayControl

// Initialize the MegaActuatorController instance
MegaActuatorController actuatorController(relays, leds);

MegaInputManager inputManager;  // Create an instance of MegaInputManager

void setup() {
    // Setup code here, if needed
    Serial.begin(115200);  // Start serial communication at 9600 baud
    Serial2.begin(115200);   // Serial communication with ESP-32
   // Serial2 uses RX (Pin 17) and TX (Pin 16) on Arduino Mega 2560
    relays.initializeRelays(); // Initialize all relays to off
    // create a clear beginning of program execution
        // Initialize switch pins

 //   switches[1] = Switch(9);
 //   switches[2] = Switch(7);
//    switches[3] = Switch(6);
    Serial.print ("\n\n/**\n/**\n/**  Version: ");
    Serial.println (KitchenScriptVersion);
    Serial.println ("/**  Script restarted on Mega board.\n/**\n/**\n");
}

void loop() {
    ButtonState extendButtonState = inputManager.extendButton.getButtonState();
    ButtonState retractButtonState = inputManager.retractButton.getButtonState();
    if (extendButtonState == ButtonState::DOUBLE_PRESSED) {
        Serial.println("\nDouble-press detected on extend button");
        relays.forceOperation(1); // true for extend
        return;
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
        return;
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
for (int i = 0; i < relays.getTotalActuators() * 2; ++i) {
    ButtonState currentState = inputManager.getSwitchState(i);  // Get the current state of the switch
    if (inputManager.getSwitchState(i) == ButtonState::SINGLE_PRESSED) {
        // Check if this is a double-flick indicating a FORCE Extend/Retract command.
        if (inputManager.isSwitchDoubleFlick(i) == ButtonState::DOUBLE_PRESSED) {
            Serial.print("Force Extend/Retract command detected on switch on pin ");
            Serial.println(i);
            // extend or retract this only this pin using the force
            int actuatorNum;
            actuatorNum = i / 2; // converts switch pins to actuator pins
            bool isExtend = !(i & 1); // checks if switch pin is even (true) or odd (false)
            relays.activate(actuatorNum,isExtend);  // Send true to activate so it doesn't convert the input pin before actuating.
            return;
        }
        Serial.print(" (index ");
        Serial.print(i);
        Serial.println(") state changed");

        // Example: Control actuators based on switch state
        if (currentState == ButtonState::SINGLE_PRESSED) {
            // Activate the corresponding actuator
            Serial.print("Activating actuator for switch on pin ");
            Serial.println(i);
            bool thisActuatorMode = inputManager.getSwitchMode(i) == SwitchMode::EXTEND;
            relays.controlSingleActuator(i, thisActuatorMode);
        } else {
            // Pause or deactivate the corresponding actuator
            Serial.print("Pausing actuator for switch on pin ");
            Serial.println(i);
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
