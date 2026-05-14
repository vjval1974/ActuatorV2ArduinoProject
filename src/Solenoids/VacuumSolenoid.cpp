#include "SolenoidState.h"
#include "SolenoidCommand.h"
#include "VacuumSolenoid.h"
#include <Arduino.h>

VacuumSolenoid::VacuumSolenoid(int outputPin)
{
    _outputPin = outputPin;
    pinMode(outputPin, OUTPUT);
}

SolenoidState VacuumSolenoid::GetState() const
{
    return digitalRead(_outputPin) == HIGH ? SolenoidState::ACTIVATED : SolenoidState::DEACTIVATED;
}

void VacuumSolenoid::Command(SolenoidCommand command)
{
    if (command == SolenoidCommand::ACTIVATE)
    {
        digitalWrite(_outputPin, LOW);
    }
    else
    {
        digitalWrite(_outputPin, HIGH);
    }
}
