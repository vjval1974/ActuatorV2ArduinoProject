
#include "MotorCommand.h"
#include "MotorState.h"
#include "Motor.h"
#include <Arduino.h>

static MotorState state;
#define PWM_FAST 90
#define PWM_SLOW 20

MotorController::MotorController(int fwPin, int bwPin, int stopPin, int speed2Pin, int faultPin)
{
    _fwPin = fwPin;
    _bwPin = bwPin;
    _stopPin = stopPin;
    _speed2Pin = speed2Pin;
    _faultPin = faultPin;

    pinMode(_fwPin, OUTPUT);
    digitalWrite(_fwPin, HIGH);
    pinMode(_bwPin, OUTPUT);
    digitalWrite(_bwPin, HIGH);
    pinMode(_stopPin, OUTPUT);
    digitalWrite(_stopPin, HIGH);
    pinMode(_speed2Pin, OUTPUT);
    digitalWrite(_speed2Pin, HIGH);
    pinMode(_faultPin, INPUT_PULLUP);
}

MotorState MotorController::GetMotorState()
{
    // todo: what happens if the controller returns a fault?

    return state;
}

bool MotorController::HasFault()
{
    return digitalRead(_faultPin);
}

void MotorController::MotorDrive(MotorCommand command)
{
    switch (command)
    {
    case DRIVE_UP_FAST:
        // set dir to up
        digitalWrite(_fwPin, LOW);
        digitalWrite(_bwPin, HIGH);
        // set enabled to true
        digitalWrite(_stopPin, HIGH);
        // set speed
        state = MOTOR_DRIVING_UP;
        digitalWrite(_speed2Pin, HIGH);
        break;
    case DRIVE_UP_SLOW:
        // set dir to up
        digitalWrite(_fwPin, LOW);
        digitalWrite(_bwPin, HIGH);
        // set enabled to true
        digitalWrite(_stopPin, HIGH);
        // set speed
        state = MOTOR_DRIVING_UP;
        digitalWrite(_speed2Pin, LOW);
        state = MOTOR_DRIVING_UP;
        break;
    case DRIVE_DOWN_FAST:
        // set dir to down
        // set enabled to true
        // set speed
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, LOW);
        // set enabled to true
        digitalWrite(_stopPin, HIGH);
        // set speed
        digitalWrite(_speed2Pin, HIGH);
        state = MOTOR_DRIVING_DOWN;
        break;
    case DRIVE_DOWN_SLOW:
        // set dir to down
        // set enabled to true
        // set speed
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, LOW);
        // set enabled to true
        digitalWrite(_stopPin, HIGH);
        // set speed
        digitalWrite(_speed2Pin, LOW);
        state = MOTOR_DRIVING_DOWN;
        break;
    case MOTOR_STOP:
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, HIGH);
        digitalWrite(_stopPin, HIGH);
        digitalWrite(_speed2Pin, HIGH);
        state = MOTOR_STOPPED;

    default:
        break;
    }
}
