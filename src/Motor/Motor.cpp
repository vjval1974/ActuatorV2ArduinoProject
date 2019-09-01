
#include "MotorCommand.h"
#include "MotorState.h"
#include "Motor.h"

static MotorState state;
#define PWM_FAST 90
#define PWM_SLOW 20

MotorState GetMotorState()
{
    // todo: what happens if the controller returns a fault?

    return state;
}

void MotorDrive(MotorCommand command)
{
    switch (command)
    {
    case DRIVE_UP_FAST:
        // set dir to up
        // set enabled to true
        // set speed
        state = MOTOR_DRIVING_UP;
        break;
    case DRIVE_UP_SLOW:
        // set dir to up
        // set enabled to true
        // set speed
        state = MOTOR_DRIVING_UP;
        break;
    case DRIVE_DOWN_FAST:
        // set dir to down
        // set enabled to true
        // set speed
        state = MOTOR_DRIVING_DOWN;
        break;
    case DRIVE_DOWN_SLOW:
        // set dir to down
        // set enabled to true
        // set speed
        state = MOTOR_DRIVING_DOWN;
        break;
    case MOTOR_STOP:
        state = MOTOR_STOPPED;

    default:
        break;
    }
}
