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
            relayStates[i / 2].remainingTime = totalDuration;
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
            if (action || !relayStates[index].remainingTime) {
              relayStates[index].remainingTime = (relayStates[index].remainingTime > 0) ? relayStates[index].remainingTime : totalDuration;
            } else { // rectify remaining time from extend to retract
              Serial.print (relayStates[index].remainingTime);
              relayStates[index].remainingTime = (relayStates[index].remainingTime > 0) ? (totalDuration - relayStates[index].remainingTime) : totalDuration;
              Serial.print ("rectified to: ");
              Serial.println (relayStates[index].remainingTime);
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
                Serial.print (relayStates[i].remainingTime);
                  relayStates[i].remainingTime = totalDuration - relayStates[i].remainingTime;
                                Serial.print ("rectified to: ");
              Serial.println (relayStates[i].remainingTime);
                }
            }
            digitalWrite(relayPins[2 * i], HIGH);
            digitalWrite(relayPins[2 * i + 1], HIGH);
            relayStates[i].isActive = false;
  //          Serial.print ("Remainging Time: ");
  //          Serial.println (i);
  //          Serial.println (relayStates[i].remainingTime);
        }
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







// Button class to handle button state changes
class Button {
public:
    Button(int pin) : pin(pin), lastState(HIGH) {
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
const int extendLedPin = 13; // 
const int retractLedPin = 10; // 
LEDControl leds(extendLedPin, retractLedPin); // Create an instance of LEDControl

const int extendButtonPin = 12; // Example pin number for the extend button
const int retractButtonPin = 11; // Example pin number for the retract button
Button extendButton(extendButtonPin);  // Creating an instance of Button for extend
Button retractButton(retractButtonPin);  // Creating an instance of Button for retract

const int relayPins[] = {51, 49, 47, 45, 43, 41, 39, 37};  // actuator relay pinout with extend being first half and retract second half
RelayControl relays(relayPins, sizeof(relayPins) / sizeof(relayPins[0]));  // Creating an instance of RelayControl

void setup() {
    // Setup code here, if needed
    Serial.begin(9600);  // Start serial communication at 9600 baud
    relays.initializeRelays(); // Initialize all relays to off

}

void loop() {
    int simulatedHour = (millis() / 200) % 24;  // Simulate time for demonstration
    if (extendButton.stateChanged()) {
      if (relays.anyActive()) {
        relays.pauseAll();
        leds.setFullBrightness(false, false);
      } else {
        relays.controlRelays(true);  // Extend relays
        leds.setFullBrightness(true, true);
      }
    } else if (retractButton.stateChanged()) {
      if (relays.anyActive()) {
        relays.pauseAll();
        leds.setFullBrightness(false, false);
      } else {
 //       Serial.println("State changed to retract");
        relays.controlRelays(false);  // Retract relays
        leds.setFullBrightness(true, false);
      }
    } 
     if (!relays.anyActive()) {
        leds.updateBlink(millis());
      }
    
    bool wasActive = relays.anyActive();
    relays.update();  // Update relay states
    if (wasActive != relays.anyActive()) {
      leds.setFullBrightness(false, false);
    }
}
