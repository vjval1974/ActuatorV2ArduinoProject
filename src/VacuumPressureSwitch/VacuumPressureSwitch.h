#ifndef VACUUMPRESSURESWITCH_H
#define VACUUMPRESSURESWITCH_H

#include "PressureSwitchState.h"

class VacuumPressureSwitch 
{
    private:
    int _inputPin;

    public: 
    VacuumPressureSwitch(int inputPin);
    bool HasVacuum();

};
//PressureSwitchState GetVacuumPressureSwitchState();

#endif