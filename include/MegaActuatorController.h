//
// Created by fredr on 3/13/2025.
//
#pragma once
#include <Arduino.h>
#include "MegaCommand.h"
#include "MegaRelayControl.h"
#include "MegaLEDControl.h"

namespace ActuatorsController {

class MegaActuatorController {
public:
  MegaActuatorController(inputMappings::MegaRelayControl
                         & relayControl, MegaLEDControl& ledControl)
    : relays(relayControl), leds(ledControl) {}

  void executeCommand(const ActuatorsController::MegaCommand& command) {
    Serial.print ("Command: ");
    Serial.println (command.getAction());
    if (command.getAction() == "EXTEND") {
      if (command.getActuator() == 0) {
        Serial.println ("EXTENDING ALL");
        relays.controlRelays(true);
      } else {
        relays.controlSingleActuator(command.getActuator());
        Serial.print ("EXTENDING: ");
        Serial.println (command.getActuator());
      }
      leds.setFullBrightness(true, true);
    } else if (command.getAction() == "RETRACT") {
      if (command.getActuator() == 0) {
        relays.controlRelays(false);
        Serial.println ("RETRACTING ALL");
      } else {
        relays.controlSingleActuator(command.getActuator());
        Serial.print ("RETRACTING: ");
        Serial.println (command.getActuator());
      }
      leds.setFullBrightness(true, false);
    }
  }

private:
  inputMappings::MegaRelayControl
& relays;
  MegaLEDControl& leds;
};
} // namespace ActuatorsController