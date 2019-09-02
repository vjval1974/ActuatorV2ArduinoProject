#ifndef VACUUMSOLENOID_H
#define VACUUMSOLENOID_H

class VacuumSolenoid
{
private:
    int _outputPin;

public:
    VacuumSolenoid(int outputPin);
    SolenoidState GetState();
    void Command(SolenoidCommand command);
};

// SolenoidState GetVacuumSolenoidState();
// void VacuumSolenoid(SolenoidCommand command);

#endif