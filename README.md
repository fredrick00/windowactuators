# windowactuators

This project uses arduino boards to control a 12v relay board to extend or retract a series of actuators which originally controlled 4 windows in a kitchen.  While the cheap actuators on amazon.com require you hold the switch until they are fully extended or retracted (30 seconds for the ones I bought), this script allows a single toggle of the switch, press of the button, or selection from the website or app to fully extend or retract the actuator pausing if the same command is given to that actuator again.  
With this script, all windows can be opened, closed, or paused simultaneously by hitting the appropriate button, or individual windows can be set to extend or retract by using a switch designated for that window.  This script controls LED lights which change states based on whether the windows are extending, retracting, or dormant.  In the original installation, these LEDs are built into the extend and retract buttons, but it could be paired with any LEDs or relays for LEDs of suitable voltage and configuration.  

The original installation of this script is on a Mega 2560 board connected to an ESP32 board and a 12v 24 circuit relay.  

Total Original Hardware Composition (ignoring wiring)
  (2) one red, one blue: APIELE 19mm Latching Push Button Switch 12V DC Angel Eye Halo Ring LED Metal 0.74" 1NO1NC SPDT with Wire Socket Plug
  (4) momentary toggle switches of various makes.  
  (1) Arduino Mega 2560 (REV3)
  (1) ESP-32 development board
  (1) breadboard
  (1) 16 circuit 12v relay board
