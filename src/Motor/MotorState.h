#ifndef MOTORSTATE_H
#define MOTORSTATE_H

typedef enum {
    MOTOR_DRIVING_UP,
    MOTOR_DRIVING_DOWN,
    MOTOR_STOPPED, 
    MOTOR_FAULT
} MotorState;

#endif