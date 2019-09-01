#include <stdio.h>
#include "../Motor/MotorState.h"
#include "../Motor/MotorCommand.h"
#include "../Motor/Motor.h"
#include "../Pushbuttons/PushbuttonState.h"
#include "../Pushbuttons/DownPushbutton.h"
#include "../Pushbuttons/UpPushbutton.h"
#include "../Pushbuttons/StopPushbutton.h"
#include "../Solenoids/SolenoidState.h"
#include "../Solenoids/SolenoidCommand.h"
#include "../Solenoids/SuctionCupSolenoid.h"
#include "../Solenoids/VacuumSolenoid.h"
#include "../Fsr/FsrState.h"
#include "../Fsr/Fsr.h"
#include "../VacuumPressureSwitch/PressureSwitchState.h"
#include "../VacuumPressureSwitch/VacuumPressureSwitch.h"
#include "../Solenoids/SuctionCupPosition.h"
#include "../Solenoids/SuctionCupPositionSwitch.h"
#include "DriveUpStateMachine.h"

typedef enum
{
	STOPPED,
	ERROR,
	DRIVING_UP,
	BOARD_SENSED,
	TUNING_UP,
	TUNING_DOWN,
	AT_BOARD,
	RAISING_CUP,
	APPLYING_VACUUM,
	IN_POSITION
} DrivingUpState;

DrivingUpState state = STOPPED;

// Sets outputs to safe state
void ResetActions()
{
	MotorDrive(MOTOR_STOP);
	VacuumSolenoid(DEACTIVATE);
	SuctionCupSolenoid(DEACTIVATE);
}

void DriveUpStateMachine()
{
	switch (state)
	{
	case STOPPED:
		//Action
		ResetActions();
		//Transition Conditions
		if (GetUpPushbuttonState() == PB_DOWN &&
			GetSuctionCupSolenoidState() == DEACTICTIVATED &&
			GetVacuumSolenoidState() == DEACTICTIVATED &&
			GetFsrState() == BELOW_LOWER_THRESHOLD &&
			GetMotorState() != MOTOR_FAULT)
		{
			state = DRIVING_UP;
		}
		break;
	case DRIVING_UP:
		//Action
		MotorDrive(DRIVE_UP_FAST);

		//Transition Conditions
		if (GetFsrState() == ABOVE_LOWER_THRESHOLD)
		{
			state = BOARD_SENSED;
		}
		if (GetFsrState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
		}
		break;

	case BOARD_SENSED:
		// Action
		MotorDrive(MOTOR_STOP);

		// Transition Conditions
		if (GetFsrState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
		}
		if (GetFsrState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
		}
		break;
	case TUNING_DOWN:
		// Action
		MotorDrive(DRIVE_DOWN_SLOW);
		// Transition Conditions
		if (GetFsrState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
		}
		if (GetFsrState() == BELOW_LOWER_THRESHOLD)
		{
			state = TUNING_UP;
		}
		break;
	case TUNING_UP:
		// Action
		MotorDrive(DRIVE_UP_SLOW);

		// Transition Conditions
		if (GetFsrState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
		}
		if (GetFsrState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
		}
		break;
	case AT_BOARD:
		// Action
		MotorDrive(MOTOR_STOP);
		// Transition Condition
		// ensure that the vacuum pressure switch is off and the motor is stopped
		if (GetVacuumPressureSwitchState() == HAS_NO_VACUUM_PRESSURE && GetMotorState() == MOTOR_STOPPED)
		{
			state = RAISING_CUP;
		}

		break;
	case RAISING_CUP:
		// Action
		SuctionCupSolenoid(ACTIVATE);
		// Transition Conditions
		if (GetSuctionCupPosition() == SUCTION_CUP_RAISED)
		{
			state = APPLYING_VACUUM;
		}
		break;
	case APPLYING_VACUUM:
		// Action
		VacuumSolenoid(ACTIVATE);
		// Transition Conditions
		if (GetVacuumPressureSwitchState() == HAS_VACUUM_PRESSURE)
		{
			state = IN_POSITION;
		}
		break;
	case IN_POSITION:
		// Action
		SuctionCupSolenoid(DEACTIVATE); // so that the board is rested back on the stop
		// Transition Conditions
		
		break;

	default:
		break;
	}
}
