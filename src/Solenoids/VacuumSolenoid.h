#ifndef VACUUMSOLENOID_H
#define VACUUMSOLENOID_H

#include "SolenoidState.h"
#include "SolenoidCommand.h"

class VacuumSolenoid
{
private:
    int _outputPin;

public:
    VacuumSolenoid(int outputPin);
    VacuumSolenoid(const VacuumSolenoid&) = delete;
    VacuumSolenoid& operator=(const VacuumSolenoid&) = delete;
    SolenoidState GetState() const;
    void Command(SolenoidCommand command);
};

#endif
