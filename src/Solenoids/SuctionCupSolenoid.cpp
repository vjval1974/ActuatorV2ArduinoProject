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

static SolenoidState state;

SolenoidState SuctionCup::GetState()
{
    return state;
}

void SuctionCup::Command(SolenoidCommand command)
{
    if (command == ACTIVATE)
    {
        // set pin state to Activate

        state = ACTIVATED;
    }
    else
    {
        // set pin state to Deactivate
        state = DEACTICTIVATED;
    }
}
