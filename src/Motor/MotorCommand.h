#ifndef MOTORCOMMAND_H
#define MOTORCOMMAND_H

#include <stdint.h>

enum class MotorCommand : uint8_t {
    DRIVE_UP_FAST,
    DRIVE_UP_SLOW,
    DRIVE_DOWN_FAST,
    DRIVE_DOWN_SLOW,
    MOTOR_STOP
};

#endif
