//
// Created by fredr on 3/13/2025.
//
#pragma once
#include <Arduino.h>

namespace ActuatorsController {

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
} // namespace ActuatorsController
