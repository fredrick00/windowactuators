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

        // Call update() periodically (for example, from your main loop)
        // It checks for any state changes and tells the reporter to send a status report.
        void checkAndReport() {
          if (_relayControl.getChangedState()) {
            _reporter.sendStatusReport();
          }
        }
    private:
      MegaRelayControl &_relayControl;
      ActuatorReporter &_reporter;
};

}  // namespace ActuatorsController


