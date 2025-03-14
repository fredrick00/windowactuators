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
  inputMappings::MegaRelayControl
& relays;
  MegaLEDControl& leds;
};
} // namespace ActuatorsController