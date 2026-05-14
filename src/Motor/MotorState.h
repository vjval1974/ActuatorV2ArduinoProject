#ifndef MOTORSTATE_H
#define MOTORSTATE_H

#include <stdint.h>

enum class MotorState : uint8_t {
    MOTOR_DRIVING_UP,
    MOTOR_DRIVING_DOWN,
    MOTOR_STOPPED,
    MOTOR_FAULT
};

#endif
