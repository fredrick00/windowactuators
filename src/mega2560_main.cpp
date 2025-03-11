#ifdef __AVR_ATmega2560__
#include <Arduino.h>
#include <array>
#include "megatypes.h"
#include "inputmapping.h"


using namespace ActuatorsController;

class MegaRelayControl {
public:
    Mode stateReport;

    struct RelayState {
        bool isActive;
        Mode relayState;
        unsigned long startTime;
        unsigned long actuatorPosition;
        unsigned long maxDuration;

    } relayStates[MAX_PINS]; // Static array for relay states

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
    forcedActive = true;
    forcedStartTime = millis();
}
// -------END Force Function----------- // *****


// Initiates a forced operation for all actuators.
void forceOperation(bool isExtend) {
    Serial.print("FORCED OPERATION (all actuators): ");
    Serial.println(isExtend ? "EXTENDING" : "RETRACTING");
    for (int i = 0; i < MAX_PINS; i++) {
        if (!relayStates[i].isActive && (inputMappings[i].mode == Mode::EXTENDING) == isExtend) {
            forceOperator(i);
        }
    }
}
// -------END Force Function----------- // *****


void controlSingleActuator(int actuatorIndex) {
    if (relayStates[actuatorIndex].isActive) {
        // The actuator is active, so pause it first
        pauseSingleActuator(actuatorIndex);
    } else {
        // Activate the actuator for the desired action (extend or retract)
        activate(actuatorIndex);
    }
}

dd


    void initializeRelays() {
        for (int i = 0; i < MAX_PINS; i++) {
            pinMode(inputMappings[i].actuatorPin, OUTPUT);
            digitalWrite(inputMappings[i].actuatorPin, HIGH); // Assuming HIGH means relay off
            relayStates[i].isActive = false;
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
        for (int i = 0; i < MAX_PINS; i++) {
            // activate relays for the action indicated by isExtend
            if ((isExtend && inputMappings[i].mode == Mode::EXTENDING) ||
                (!isExtend && inputMappings[i].mode == Mode::RETRACTING)) {
                activate(i);
            }

        }
      }
    }

void activate(int actuatorIndex) {
    Serial.print("Activating actuator. (Index/Action): ");
    Serial.print(inputMappings[actuatorIndex].actuatorName);
    Serial.print("/");
    Serial.println((inputMappings[actuatorIndex].mode == Mode::EXTENDING) ? "EXTENDING" : "RETRACTING");

       // if this actuator is not active.
    if (!relayStates[actuatorIndex].isActive) {
        digitalWrite(inputMappings[actuatorIndex].actuatorPin, LOW);  // Activate the relay
        relayStates[actuatorIndex].isActive = true;
        relayStates[actuatorIndex].relayState = inputMappings[actuatorIndex].mode;
        relayStates[actuatorIndex].startTime = millis();
        Serial.print("Actuator Position: ");
        Serial.println (relayStates[actuatorIndex].actuatorPosition);
    }
}


    void pauseAll() {
        Serial.println ("Pausing all actuators.");
        for (int i = 0; i < MAX_PINS; i++) {
            pauseSingleActuator(i);
        }
    }

    bool anyActive() const {
        for (int i = 0; i < MAX_PINS; i++) {
            if (relayStates[i].isActive) {
                return true;
            }
        }
        return false;
    }

    bool areAnyExtending() const {
        for (int i = 0; i < MAX_PINS; i++) {
            if (relayStates[i].relayState == Mode::EXTENDING) {
                return true;
            }
        }
        return false;
    }

    bool areAnyRetracting() const {
        for (int i = MAX_PINS / 2; i < MAX_PINS; i++) {
            if (relayStates[i].relayState == Mode::RETRACTING) {
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

    for (int i = 0; i < MAX_PINS; i++) {
        if (relayStates[i].isActive) {
            // Calculate elapsed time
            unsigned long elapsedTime = currentTime - relayStates[i].startTime;
            // The new duration is the previous duration + for extend or - for retract the elapsed time.
            unsigned long newDuration = 0;
            if (relayStates[i].relayState == Mode::EXTENDING) {
                newDuration = relayStates[i].actuatorPosition + elapsedTime;
            } else if (relayStates[i].relayState == Mode::RETRACTING) { // retracting
                newDuration = (relayStates[i].actuatorPosition < elapsedTime) ? 0UL
                                  : relayStates[i].actuatorPosition - elapsedTime;
            }
            // Check if the relay should be turned off
            // if the newDuration is > 0 or < duration, then we keep going.
            if ((newDuration <= 0 && relayStates[i].relayState == Mode::RETRACTING)|| (newDuration > relayStates[i].maxDuration && relayStates[i].relayState == Mode::EXTENDING)) {
                    pauseSingleActuator(i);
           }

          }
        }
    }

private:
    bool activateRelay;
    unsigned long remainingTimes[MAX_PINS];

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
    unsigned long currentPressTime = millis();;
    unsigned long lastPressTime = currentPressTime;
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
    if (command.getAction() == "EXTENDING") {
      if (command.getActuator() == 0) {
        relays.controlRelays(true);
      } else {
        relays.controlSingleActuator(command.getActuator());
      }
      leds.setFullBrightness(true, true);
    } else if (command.getAction() == "RETRACTING") {
      if (command.getActuator() == 0) {
        relays.controlRelays(false);
      } else {
        relays.controlSingleActuator(command.getActuator());
      }
      leds.setFullBrightness(true, false);
    }
  }

private:
  MegaRelayControl
& relays;
  MegaLEDControl& leds;
};

// Class for handling debouncing as we check our inputs
// C++ code (e.g., for Arduino)

class DebouncedSwitch {
public:
    DebouncedSwitch() = default; // Default constructor for std::array

    // Constructor:
    // - pin: the digital pin connected to the switch.
    // - debounceInterval: minimum time in milliseconds the input must remain stable before updating the state.
    DebouncedSwitch(int pin, unsigned long debounceInterval = 50)
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
    void acknowledgeState() {
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



class ActuatorReporter {
public:
    // Constructor: stores a reference to the MegaRelayControl which holds relay and state information.
    ActuatorReporter(MegaRelayControl &relayControl)
        : relays(relayControl), lastReportTime(millis()) {}

    // Virtual destructor (if you later subclass this reporter)
    virtual ~ActuatorReporter() {}

    // Generates a JSON-formatted report string with current actuator states.
    String generateReport() const {
        String report = "{ \"timestamp\": " + String(millis());
        report += ", \"forceMode\": " + String(relays.isForceMode() ? "true" : "false"); // Report forced mode
        report += ", \"actuators\": [";
        for (int i = 0; i < MAX_PINS; i++) {
            const auto &state = relays.relayStates[i];
            report += "{ \"index\": " + String(i);
            report += ", \"active\": " + String(state.isActive ? "true" : "false");
            report += ", \"mode\": \"";
            if (state.isActive) {


                report += (state.relayState != Mode::NONE ? state.relayState == Mode::EXTENDING ? "EXTENDING" : "RETRACTING" : "IDLE");
            } else {
                report += "IDLE";
            }
            report += "\", \"position\": " + String(state.actuatorPosition);
            report += ", \"maxDuration\": " + String(state.maxDuration);
            report += "}";
            if (i < MAX_PINS - 1) {
                report += ", ";
            }
        }
        report += "] }";
        return report;
    }

    // Sends the report via Serial2 (assumed to be used for communication with the ESP32).
    void sendStatusReport() const {
        String reportString = generateReport();
        Serial2.println(reportString);
    }

    // Call this function periodically (e.g., from loop()).
    // It will send a new report if the reporting interval has elapsed.
    void update() {
        unsigned long currentTime = millis();
        if ((currentTime - lastReportTime) >= REPORT_INTERVAL) {
            sendStatusReport();
            lastReportTime = currentTime;
        }
    }

private:
    // Reference to the MegaRelayControl instance, from which we retrieve actuator states.
    MegaRelayControl &relays;
    // Timestamp of the last report sent.
    unsigned long lastReportTime;
    // Reporting interval in milliseconds (adjust as needed).
    static const unsigned long REPORT_INTERVAL = 1000UL;
};


//-------------------------------------------------------------------- //
// MegaInputManager class: Reads two overall buttons and an array // of switch inputs (assumed here to be four physical switches).
class MegaInputManager {


private:
    // Hardware buttons for overall extend/retract actions.
    unsigned long thisSwitchActivationTime[MAX_PINS];
    unsigned long previousSwitchActivationTime[MAX_PINS];




    public:
    // Use the same enum for button and switch events.
    MegaButton extendButton;
    MegaButton retractButton;
    Switch switches[MAX_PINS]; // Use the same type as the events produced by buttons.
    ButtonState switchStates[MAX_PINS];
    // State variables for the extend and retract buttons.
    ButtonState extendState;
    ButtonState retractState;

    // Constructor: Create the two buttons and initialize the switch array.
    MegaInputManager()
    : extendButton(inputMappings[EXTEND_BUTTON_INDEX].inputPin),
      retractButton(inputMappings[RETRACT_BUTTON_INDEX].inputPin),
      extendState(ButtonState::NONE),
      retractState(ButtonState::NONE)
    {
        // Initialize each physical switch with its corresponding pin.
        for (int i = 0; i < MAX_PINS; i++) {
            switches[i] = Switch(inputMappings[i].inputPin);
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
        for (int i = 0; i < MAX_PINS; i++) {
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
        return (index >= 0 && index < MAX_PINS) ? switchStates[index] : ButtonState::NONE;
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





// Pin setup and object instantiation
const int extendLedPin = 11; // 
const int retractLedPin = 10; // 
MegaLEDControl leds(extendLedPin, retractLedPin); // Create an instance of MegaLEDControl


//const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // actuator relay pinout with extend being first half and retract second half
MegaRelayControl relays;  // Creating an instance of MegaRelayControl

// Initialize the MegaActuatorController instance
MegaActuatorController actuatorController(relays, leds);

MegaInputManager inputManager;  // Create an instance of MegaInputManager
ActuatorReporter* statusReporter = nullptr;

// Using std::array for fixed-size allocation.
std::array<DebouncedSwitch, MAX_INPUTS_COUNT> debouncedSwitches;

// Create an instance (adjust the pin and interval as needed)
DebouncedSwitch mySwitch(2, 50); // Pin 2 with 50ms debounce time

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
