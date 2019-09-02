#ifndef MOTOR_H
#define MOTOR_H
#include "MotorCommand.h"
#include "MotorState.h"

class MotorController
{
    private: 
    int _fwPin; 
    int _bwPin;
    int _stopPin; 
    int _speed2Pin;
    int _faultPin; 

    public: 
    MotorController(int fwPin, int bwPin, int stopPin, int speed2Pin, int faultPin);
    
    void MotorDrive(MotorCommand command);
    MotorState GetMotorState();
    bool HasFault();
};

#endif