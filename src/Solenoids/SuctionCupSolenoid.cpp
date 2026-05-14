#include "SuctionCupSolenoid.h"
#include <Arduino.h>

SuctionCup::SuctionCup(int outputPin, int positionInputPin)
{
    _positionInputPin = positionInputPin;
    _outputPin = outputPin;
    pinMode(_outputPin, OUTPUT);
    pinMode(_positionInputPin, INPUT_PULLUP);
}

SuctionCupPosition SuctionCup::GetPosition() const
{
    return digitalRead(_positionInputPin) == HIGH
               ? SuctionCupPosition::SUCTION_CUP_RAISED
               : SuctionCupPosition::SUCTION_CUP_LOWERED;
}

SolenoidState SuctionCup::GetState() const
{
    return _state;
}

void SuctionCup::Command(SolenoidCommand command)
{
    if (command == SolenoidCommand::ACTIVATE)
    {
        digitalWrite(_outputPin, HIGH);
        _state = SolenoidState::ACTIVATED;
    }
    else
    {
        digitalWrite(_outputPin, LOW);
        _state = SolenoidState::DEACTIVATED;
    }
}
