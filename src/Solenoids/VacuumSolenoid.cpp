#include "SolenoidState.h"
#include "SolenoidCommand.h"
#include "VacuumSolenoid.h"



static SolenoidState state;

SolenoidState GetVacuumSolenoidState()
{
    return state;
}

void VacuumSolenoid(SolenoidCommand command)
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
