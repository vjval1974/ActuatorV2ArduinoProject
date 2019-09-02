#include "SolenoidState.h"
#include "SolenoidCommand.h"
#include "VacuumSolenoid.h"
#include <Arduino.h>

VacuumSolenoid::VacuumSolenoid(int outputPin)
{
    _outputPin = outputPin;
    pinMode(outputPin, OUTPUT);
}

SolenoidState VacuumSolenoid::GetState()
{
    return digitalRead(_outputPin) == LOW ? ACTIVATED : DEACTICTIVATED;
}

void VacuumSolenoid::Command(SolenoidCommand command)
{
    if (command == ACTIVATE)
    {
        digitalWrite(_outputPin, LOW);
        // set pin state to Activate
    
    }
    else
    {
        pinMode(_outputPin, HIGH);
        // set pin state to Deactivate
    
    }
}
