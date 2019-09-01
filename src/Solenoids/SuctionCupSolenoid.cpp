#include "SolenoidState.h"
#include "SolenoidCommand.h"
#include "SuctionCupSolenoid.h"

static SolenoidState state;

SolenoidState GetSuctionCupSolenoidState()
{
    return state;
}

void SuctionCupSolenoid(SolenoidCommand command)
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
