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

public:
    SuctionCup(int outputPin, int positionInputPin);
    SuctionCupPosition GetPosition();
    SolenoidState GetState();
    void Command(SolenoidCommand command);
};

// SolenoidState GetSuctionCupSolenoidState();
// void SuctionCupSolenoid(SolenoidCommand command);

#endif