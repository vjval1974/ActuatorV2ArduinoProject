#include "VacuumPressureSwitch.h"
#include "PressureSwitchState.h"
#include <Arduino.h>


VacuumPressureSwitch::VacuumPressureSwitch(int inputPin)
{
    _inputPin = inputPin;
    pinMode(_inputPin, INPUT_PULLUP);
}

bool VacuumPressureSwitch::HasVacuum()
{
    return digitalRead(_inputPin) == HIGH;
}

