#include <Arduino.h>
#include "./StateMachines/ActuatorStateMachine.h"

/*
Pro Micro silkscreen pin -> function. Numeric values used throughout the code
correspond to the silkscreen labels (e.g. digitalWrite(8, ...)).

ASSIGNMENTS
============
PIN  2   Motor Controller fwd
PIN  3   Motor Controller bwd
PIN  4   Motor Controller stop
PIN  5   Motor Controller speed2
PIN  6   Motor Controller fault
PIN  8   Suction Cup solenoid
PIN  9   Suction Cup position sensor
PIN 10   Vacuum Pressure switch
PIN 14   Vacuum Solenoid
PIN A0   Start Pushbutton
PIN A1   Stop Pushbutton
PIN A2   Touch Sensor
*/

static ActuatorStateMachine actuatorStateMachine;

void setup()
{
	Serial.begin(115200);
	delay(1000);
	Serial.println(F("Started"));
}

void loop()
{
	actuatorStateMachine.Process();
	delay(20);
}
