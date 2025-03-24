//
// Created by fredr on 3/14/2025.
//

#pragma once
#include "MegaRelayControl.h"
#include "ActuatorReporter.h"


namespace ActuatorsController {

class MegaStateWatcher {
    public:
        // Constructor: pass in references to the relay control and reporter instances
        MegaStateWatcher(MegaRelayControl &relayControl, ActuatorReporter &reporter)
            : _relayControl(relayControl), _reporter(reporter) { }

        // It checks for any state changes and tells the reporter to send a status report.
        void checkAndReport() {
          if (_relayControl.getChangedState()) {
            // iterate through the relays generating a report for each relay which experienced a state change.
            for (int i = 0; i < MAX_RELAY_PINS; i++) {
              if (_relayControl.hasRelayChangedState(i)) {
                _reporter.sendStatusReport(i);
                // reset the relay changed state now that we've generated the report.
                _relayControl.setRelayChangedState(i, false);
              }
            }
          }
        }
    private:
      MegaRelayControl &_relayControl;
      ActuatorReporter &_reporter;
};

}  // namespace ActuatorsController


