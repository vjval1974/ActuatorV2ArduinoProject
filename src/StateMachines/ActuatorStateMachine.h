#ifndef ACTUATORSTATEMACHINE_H
#define ACTUATORSTATEMACHINE_H

#include "../VacuumPressureSwitch/VacuumPressureSwitch.h"
#include "../Solenoids/SuctionCupSolenoid.h"
#include "../Solenoids/VacuumSolenoid.h"
#include "../Pushbuttons/FsrPushbutton.h"
#include "../Motor/Motor.h"
#include "../Fsr/TouchSensor.h"


typedef enum
{
	STOPPED,
	ERROR,
	DRIVING_UP,
	DRIVING_DOWN,
	BOARD_SENSED,
	TUNING_UP,
	TUNING_DOWN,
	AT_BOARD,
	RAISING_CUP,
	APPLYING_VACUUM,
	IN_POSITION
} ActuatorState;

class ActuatorStateMachine {
    private:
    FsrPushbutton startPushbutton = FsrPushbutton(A0, 5);
	FsrPushbutton stopPushbutton = FsrPushbutton(A1, 5);
	TouchSensor touchSensor = TouchSensor(A2, 20, 80);
	MotorController actuatorMotorController = MotorController(PIN1, PIN2, PIN3, PIN4, PIN5);
	SuctionCup suctionCup = SuctionCup(PIN6, PIN7);
	VacuumPressureSwitch vacuumPressureSwitch = VacuumPressureSwitch(PINB0);
	VacuumSolenoid vacuumSolenoid = VacuumSolenoid(PINB1);
    ActuatorState state;
	ActuatorState previousState;

    void ResetActions();
	static bool ShouldTransitionOnPress(PressState state);
    static void PrintStateTransition(ActuatorState state, ActuatorState previousState, TouchSensor sensor);

public:
    ActuatorStateMachine();
    void Process();
};

//void DriveUpStateMachine();

#endif