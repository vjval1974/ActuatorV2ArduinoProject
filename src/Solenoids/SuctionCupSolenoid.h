#ifndef SUCTIONCUPSOLENOID_H
#define SUCTIONCUPSOLENOID_H

#include "SuctionCupPosition.h"
#include "SolenoidState.h"
#include "SolenoidCommand.h"

class SuctionCup
{
private:
    int _outputPin;
    int _positionInputPin;
    SolenoidState _state;

public:
    SuctionCup(int outputPin, int positionInputPin);
    SuctionCup(const SuctionCup&) = delete;
    SuctionCup& operator=(const SuctionCup&) = delete;
    SuctionCupPosition GetPosition() const;
    SolenoidState GetState() const;
    void Command(SolenoidCommand command);
};

#endif
