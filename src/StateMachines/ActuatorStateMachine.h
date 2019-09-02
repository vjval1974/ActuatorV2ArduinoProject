#ifndef DRIVEUPSTATEMACHINE_H
#define DRIVEUPSTATEMACHINE_H

#include "../Pushbuttons/FsrPushbutton.h"
#include "../Motor/Motor.h"
#include "../Fsr/TouchSensor.h"

class ActuatorStateMachine {
    private:
    FsrPushbutton startPushbutton = FsrPushbutton(A1, 5);
    FsrPushbutton stopPushbutton = FsrPushbutton(A2, 5);
    TouchSensor touchSensor = TouchSensor(A3, 20, 80);
    MotorController actuatorMotorController = MotorController(PIN1, PIN2, PIN3, PIN4, PIN5);
    
    void ResetActions();

public:
    ActuatorStateMachine();
    void Process();
};

void DriveUpStateMachine();

#endif