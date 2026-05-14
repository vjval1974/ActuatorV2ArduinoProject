#include "MotorCommand.h"
#include "MotorState.h"
#include "Motor.h"
#include <Arduino.h>

MotorController::MotorController(int fwPin, int bwPin, int stopPin, int speed2Pin, int faultPin)
{
    _fwPin = fwPin;
    _bwPin = bwPin;
    _stopPin = stopPin;
    _speed2Pin = speed2Pin;
    _faultPin = faultPin;
    _state = MotorState::MOTOR_STOPPED;
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

// default constructor — BROKEN delegation (see CLAUDE.md "Known bugs").
// Calls MotorController(2,3,4,5,6) as a plain expression, constructing a
// discarded temporary; pinMode() never fires on the motor pins. Preserved
// as-is to keep hardware behaviour unchanged.
MotorController::MotorController()
{
    MotorController(2, 3, 4, 5, 6);
}

MotorState MotorController::GetMotorState() const
{
    return _state;
}

bool MotorController::HasFault() const
{
    return digitalRead(_faultPin);
}

void MotorController::MotorDrive(MotorCommand command)
{
    switch (command)
    {
    case MotorCommand::DRIVE_UP_FAST:
        digitalWrite(_fwPin, LOW);
        digitalWrite(_bwPin, HIGH);
        digitalWrite(_stopPin, HIGH);
        digitalWrite(_speed2Pin, HIGH);
        _state = MotorState::MOTOR_DRIVING_UP;
        break;
    case MotorCommand::DRIVE_UP_SLOW:
        digitalWrite(_fwPin, LOW);
        digitalWrite(_bwPin, HIGH);
        digitalWrite(_stopPin, HIGH);
        digitalWrite(_speed2Pin, LOW);
        _state = MotorState::MOTOR_DRIVING_UP;
        break;
    case MotorCommand::DRIVE_DOWN_FAST:
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, LOW);
        digitalWrite(_stopPin, HIGH);
        digitalWrite(_speed2Pin, HIGH);
        _state = MotorState::MOTOR_DRIVING_DOWN;
        break;
    case MotorCommand::DRIVE_DOWN_SLOW:
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, LOW);
        digitalWrite(_stopPin, HIGH);
        digitalWrite(_speed2Pin, LOW);
        _state = MotorState::MOTOR_DRIVING_DOWN;
        break;
    case MotorCommand::MOTOR_STOP:
        digitalWrite(_fwPin, HIGH);
        digitalWrite(_bwPin, HIGH);
        digitalWrite(_stopPin, HIGH);
        digitalWrite(_speed2Pin, HIGH);
        _state = MotorState::MOTOR_STOPPED;
        break;
    }
}
