#include "SuctionCupSolenoid.h"
#include <Arduino.h>

SuctionCup::SuctionCup(int outputPin, int positionInputPin)
{
    _positionInputPin = positionInputPin;
    _outputPin = outputPin;
    pinMode(_outputPin, OUTPUT);
    pinMode(_positionInputPin, INPUT_PULLUP);
}

SuctionCupPosition SuctionCup::GetPosition()
{
    return digitalRead(_positionInputPin) == HIGH
               ? SUCTION_CUP_RAISED
               : SUCTION_CUP_LOWERED;
}



SolenoidState SuctionCup::GetState()
{
    return _state;
}

void SuctionCup::Command(SolenoidCommand command)
{
    if (command == ACTIVATE)
    {
        // set pin state to Activate
        digitalWrite(_outputPin, HIGH);
        _state = ACTIVATED;
    }
    else
    {
        // set pin state to Deactivate
        
        digitalWrite(_outputPin, LOW);
        _state = DEACTICTIVATED;
    }
}
