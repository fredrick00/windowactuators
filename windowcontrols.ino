#include <Arduino.h>

class RelayControl {
public:
    static const int MAX_PINS = 8;  // Maximum number of relays controlled
    int relayPins[MAX_PINS];        // Array to hold relay pin numbers
    struct RelayState {
        bool isActive;
        bool isExtending;
        bool isRetracting;
        unsigned long startTime;
        unsigned long remainingTime;
        unsigned long duration; // Duration for each actuator's motion
    } relayStates[MAX_PINS / 2];    // Array for relay states

    RelayControl(const int* pins, int numPins) : numPins(numPins) {
        for (int i = 0; i < numPins; i++) {
            relayPins[i] = pins[i];
        }
        initializeRelays();
    }

    void initializeRelays() {
        for (int i = 0; i < numPins; i += 2) {
            pinMode(relayPins[i], OUTPUT);
            pinMode(relayPins[i + 1], OUTPUT);
            digitalWrite(relayPins[i], HIGH);
            digitalWrite(relayPins[i + 1], HIGH);
            relayStates[i / 2].isActive = false;
            relayStates[i / 2].isExtending = false;
            relayStates[i / 2].isRetracting = false;
            relayStates[i / 2].startTime = 0;
            relayStates[i / 2].remainingTime = 0;
            relayStates[i / 2].duration = (i < 4) ? 11000 : 30000; // First two actuators 11 seconds, last two 30 seconds
        }
    }

    void controlRelays(bool isExtend) {
        if (anyActive()) {
            pauseAll();
            return;
        }
        int actuatorCount = numPins / 4; // Each actuator has two modes: extend and retract
        for (int i = 0; i < actuatorCount; i++) {
            activate(i, isExtend);
        }
    }

    void activate(int index, bool action) {
        int pinIdx = action ? index * 2 : index * 2 + 1; // Select the correct pin based on action
        if (!relayStates[index].isActive) {
            digitalWrite(relayPins[pinIdx], LOW); // Activate the relay
            relayStates[index].isActive = true;
            relayStates[index].isExtending = action;
            relayStates[index].isRetracting = !action;
            relayStates[index].startTime = millis();

            if (action) { // If extending
                if (relayStates[index].remainingTime == 0) {
                    relayStates[index].remainingTime = relayStates[index].duration; // Set to full duration if never set
                }
            } else { // If retracting
                relayStates[index].remainingTime = relayStates[index].duration - relayStates[index].remainingTime; // Adjust remaining time
            }
        }
    }

    void pauseAll() {
        unsigned long currentTime = millis();
        for (int i = 0; i < numPins / 4; i++) {
            if (relayStates[i].isActive) {
                unsigned long elapsed = currentTime - relayStates[i].startTime;
                relayStates[i].remainingTime = max(relayStates[i].remainingTime - elapsed, 0UL);
                if (relayStates[i].isRetracting) { // rectify remainingTime from retract to extend
                    // Serial.print (relayStates[i].remainingTime);
                    relayStates[i].remainingTime = relayStates[i].duration - relayStates[i].remainingTime;
                    // Serial.print ("rectified to: ");
                    // Serial.println (relayStates[i].remainingTime);
                }
            }
            digitalWrite(relayPins[2 * i], HIGH);
            digitalWrite(relayPins[2 * i + 1], HIGH);
            relayStates[i].isActive = false;
        }
    }

    // New method to pause an individual actuator
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

    // allow checking if a single actuator is active
    bool isActuatorActive(int actuatorIndex) {
        return relayStates[actuatorIndex].isActive;
    }

    // method to control an individual actuator
    void controlSingleActuator(int actuatorIndex, bool isExtend) {
        int pinIdx = isExtend ? actuatorIndex * 2 : actuatorIndex * 2 + 1;
        if (relayStates[actuatorIndex].isActive && ((isExtend && !relayStates[actuatorIndex].isExtending) || (!isExtend && !relayStates[actuatorIndex].isRetracting))) {
            // The actuator is active but in the opposite state, so pause it first
            pauseSingleActuator(actuatorIndex);
        }
        activate(actuatorIndex, isExtend); // Call activate with the index and action
    }

    bool anyActive() const {
        for (int i = 0; i < numPins / 4; i++) {
            if (relayStates[i].isActive) return true;
        }
        return false;
    }

    void update() {
        unsigned long currentMillis = millis();
        for (int i = 0; i < numPins / 4; i++) {
            if (relayStates[i].isActive && (currentMillis - relayStates[i].startTime >= relayStates[i].remainingTime)) {
                digitalWrite(relayPins[2 * i + (relayStates[i].isExtending ? 0 : 1)], HIGH);
                relayStates[i].isActive = false;
                relayStates[i].isExtending = false;
                relayStates[i].isRetracting = false;
                relayStates[i].remainingTime = 0;  // Reset remaining time after completion
            }
        }
    }

private:
    int numPins;
    const unsigned long totalDuration = 5000; // Total duration in milliseconds
};

// Switch class to handle switch state changes
class Switch {
public:
    Switch(int pin) : pin(pin), lastState(HIGH), currentState(HIGH), lastDebounceTime(0) {
        pinMode(pin, INPUT_PULLUP);
        lastState = digitalRead(pin);  // Initialize lastState with the current state of the switch
        currentState = lastState;
    }

    bool stateChanged() {
        int reading = digitalRead(pin);

        if (reading != lastState) {
            lastDebounceTime = millis();
        }

        // Serial.print ("stateChanged: ");
        // Serial.print (pin);
        // Serial.print ("  -  ");
        // Serial.print (reading);
        // Serial.print (" / ");
        // Serial.println (lastState); 
        // delay (1000);

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (reading != currentState) {
                currentState = reading;
                return true;
            }
        }

        lastState = reading;
        return false;
    }

private:
    int pin;
    int lastState;
    int currentState;
    unsigned long lastDebounceTime;
    static const long debounceDelay = 150;  // Debounce delay in milliseconds
};

// Button class to handle button state changes
class Button {
public:
    Button(int pin) : pin(pin), lastState(HIGH), currentState(HIGH), lastDebounceTime(0) {
        pinMode(pin, INPUT_PULLUP);
        lastState = digitalRead(pin);  // Initialize lastState with the current state of the button
        currentState = lastState;
    }

    bool stateChanged() {
        int reading = digitalRead(pin);

        if (reading != lastState) {
            lastDebounceTime = millis();
        }

        // Serial.print ("button changed: (");
        // Serial.print (pin);
        // Serial.print (") ");
        // Serial.print (lastState);
        // Serial.print (" --> ");
        // Serial.println (reading);
        // delay(500);

        if ((millis() - lastDebounceTime) > debounceDelay) {
            if (reading != currentState) {
                currentState = reading;
                return true;
            }
        }

        lastState = reading;
        return false;
    }

private:
    int pin;
    int lastState;
    int currentState;
    unsigned long lastDebounceTime;
    static const long debounceDelay = 50;  // Debounce delay in milliseconds
};

// LEDControl class to handle LED operations
class LEDControl {
public:
    LEDControl(int extendLedPin, int retractLedPin) 
        : extendLedPin(extendLedPin), retractLedPin(retractLedPin), previousMillis(0), extendLedState(false), retractLedState(false), nightMode(false) {
        pinMode(extendLedPin, OUTPUT);
        pinMode(retractLedPin, OUTPUT);
        previousMillis = millis();
    }

    void updateBlink(unsigned long currentMillis) {
        if ((extendLedState && currentMillis - previousMillis >= 1000) || 
            (!extendLedState && currentMillis - previousMillis >= 3000)) {
            extendLedState = !extendLedState;
            retractLedState = !retractLedState;
            analogWrite(extendLedPin, extendLedState ? 28 : 0);  // Toggle between 50% and 0% brightness
            analogWrite(retractLedPin, retractLedState ? 28 : 0);  // Toggle between 50% and 0% brightness
            previousMillis = currentMillis;
        }
    }

    void setFullBrightness(bool on, bool isExtend) {
        if (on) {
            analogWrite(extendLedPin, isExtend ? 255 : 0); // Full brightness for extend LED
            analogWrite(retractLedPin, !isExtend ? 255 : 0); // Full brightness for retract LED
        } else {
            previousMillis = millis(); //  start the timing for blink/off cycle from here
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

// Pin setup and object instantiation
const int extendLedPin = 11; // Pin for the extend LED
const int retractLedPin = 10; // Pin for the retract LED
const int extendButtonPin = 12; // Pin number for the extend button
const int retractButtonPin = 13; // Pin number for the retract button

// Define the extend and retract pins for individual actuators
const int extendPins[] = {8, 7, 5, 3};
const int retractPins[] = {9, 6, 4, 2};


// Instantiate the switches
Switch extendSwitches[] = {
  Switch(extendPins[0]), 
  Switch(extendPins[1]), 
  Switch(extendPins[2]), 
  Switch(extendPins[3])
};

Switch retractSwitches[] = {
  Switch(retractPins[0]), 
  Switch(retractPins[1]), 
  Switch(retractPins[2]), 
  Switch(retractPins[3])
};

LEDControl leds(extendLedPin, retractLedPin); // Create an instance of LEDControl

Button extendButton(extendButtonPin);  // Creating an instance of Button for extend
Button retractButton(retractButtonPin);  // Creating an instance of Button for retract

const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // Actuator relay pinout with extend being first half and retract second half
RelayControl relays(relayPins, sizeof(relayPins) / sizeof(relayPins[0]));  // Creating an instance of RelayControl

void setup() {
    // Setup code here, if needed
    Serial.begin(9600);  // Start serial communication at 9600 baud
    relays.initializeRelays(); // Initialize all relays to off
}

void loop() {
    int simulatedHour = (millis() / 200) % 24;  // Simulate time for demonstration

    if (extendButton.stateChanged()) {
      // Serial.print ("extend button: ");
      // Serial.println (relays.anyActive());
      if (relays.anyActive()) {
        relays.pauseAll();
        leds.setFullBrightness(false, false);
      } else {
        relays.controlRelays(true);  // Extend relays
        leds.setFullBrightness(true, true);
      }
    } else if (retractButton.stateChanged()) {
        // Serial.print ("retract button: ");
        // Serial.println (relays.anyActive());
        if (relays.anyActive()) {
        relays.pauseAll();
        leds.setFullBrightness(false, false);
      } else {
        // Serial.println("State changed to retract");
        relays.controlRelays(false);  // Retract relays
        leds.setFullBrightness(true, false);
      }
    }

    // Handle extend and retract switches
    for (int i = 0; i < 4; i++) {
        if (extendSwitches[i].stateChanged()) {
            if (digitalRead(extendPins[i]) == LOW) {  // If extend switch is pressed
                if (relays.isActuatorActive(i)) {
                    relays.pauseSingleActuator(i);  // Pause if already active
                } else {
                    relays.controlSingleActuator(i, true);  // Extend actuator
                    leds.setFullBrightness(true, true);
                }
            }
        }
        if (retractSwitches[i].stateChanged()) {
            if (digitalRead(retractPins[i]) == LOW) {  // If retract switch is pressed
                if (relays.isActuatorActive(i)) {
                    relays.pauseSingleActuator(i);  // Pause if already active
                } else {
                    relays.controlSingleActuator(i, false);  // Retract actuator
                    leds.setFullBrightness(true, false);
                }
            }
        }
    } // end individual switches logic

    if (!relays.anyActive()) {
        leds.updateBlink(millis());
    }

    bool wasActive = relays.anyActive();
    relays.update();  // Update relay states
    if (wasActive != relays.anyActive()) {
      leds.setFullBrightness(false, false);
    }
}
