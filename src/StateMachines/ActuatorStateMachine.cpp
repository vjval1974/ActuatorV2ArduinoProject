#include <stdio.h>

#include "ActuatorStateMachine.h"
#define DEBUG_MODE 1

String statesText[10] =
	{"STOPPED",
	 "ERROR",
	 "DRIVING_UP",
	 "BOARD_SENSED",
	 "TUNING_UP",
	 "TUNING_DOWN",
	 "AT_BOARD",
	 "RAISING_CUP",
	 "APPLYING_VACUUM",
	 "IN_POSITION"};

void PrintStateTransition(ActuatorState state)
{
	Serial.print("Transitioning State to ");
	Serial.println(statesText[state]);
}

ActuatorStateMachine::ActuatorStateMachine()
{
	startPushbutton = FsrPushbutton(A1, 5);
	stopPushbutton = FsrPushbutton(A2, 5);
	touchSensor = TouchSensor(A3, 20, 80);
	actuatorMotorController = MotorController(PIN1, PIN2, PIN3, PIN4, PIN5);
	suctionCup = SuctionCup(PIN6, PIN7);
	vacuumPressureSwitch = VacuumPressureSwitch(PINB0);
	vacuumSolenoid = VacuumSolenoid(PINB1);
	state = STOPPED;
}

// Sets outputs to safe state
void ActuatorStateMachine::ResetActions()
{
	actuatorMotorController.MotorDrive(MOTOR_STOP);
	vacuumSolenoid.Command(DEACTIVATE);
	suctionCup.Command(DEACTIVATE);
}

void ActuatorStateMachine::Process()
{
	startPushbutton.PollPresses();
	stopPushbutton.PollPresses();
	PressState startButtonState = startPushbutton.IsPress();
	//PressState stopButtonState = stopPushbutton.IsPress();

	switch (state)
	{
	case STOPPED:
		//Action
		ResetActions();

		//Transition Conditions
		if ((startButtonState == SINGLE_PRESS &&
			 suctionCup.GetState() == DEACTICTIVATED &&
			 suctionCup.GetPosition() == SUCTION_CUP_LOWERED &&
			 vacuumSolenoid.GetState() == DEACTICTIVATED &&
			 vacuumPressureSwitch.HasVacuum() == false &&
			 touchSensor.GetState() == BELOW_LOWER_THRESHOLD &&
			 actuatorMotorController.GetMotorState() != MOTOR_FAULT)
#ifdef DEBUG_MODE
			|| startButtonState == SINGLE_PRESS
#endif
		)
		{
			state = DRIVING_UP;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;
	case DRIVING_UP:
		//Action
		actuatorMotorController.MotorDrive(DRIVE_UP_FAST);

		//Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD
#ifdef DEBUG_MODE
			|| startButtonState == SINGLE_PRESS
#endif

		)
		{
			state = BOARD_SENSED;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD
#ifdef DEBUG_MODE
			|| startButtonState == SINGLE_PRESS
#endif
		)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;

	case BOARD_SENSED:
		// Action
		actuatorMotorController.MotorDrive(MOTOR_STOP);

		// Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD
#ifdef DEBUG_MODE
			|| startButtonState == SINGLE_PRESS
#endif
		)
		{
			state = AT_BOARD;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;
	case TUNING_DOWN:
		// Action
		actuatorMotorController.MotorDrive(DRIVE_DOWN_SLOW);
		// Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		if (touchSensor.GetState() == BELOW_LOWER_THRESHOLD)
		{
			state = TUNING_UP;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;
	case TUNING_UP:
		// Action
		actuatorMotorController.MotorDrive(DRIVE_UP_SLOW);

		// Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;
	case AT_BOARD:
		// Action
		actuatorMotorController.MotorDrive(MOTOR_STOP);
		// Transition Condition
		// ensure that the vacuum pressure switch is off and the motor is stopped
		if (vacuumPressureSwitch.HasVacuum() == false && actuatorMotorController.GetMotorState() == MOTOR_STOPPED)
		{
			state = RAISING_CUP;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}

		break;
	case RAISING_CUP:
		// Action
		suctionCup.Command(ACTIVATE);
		// Transition Conditions
		if (suctionCup.GetPosition() == SUCTION_CUP_RAISED)
		{
			state = APPLYING_VACUUM;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;
	case APPLYING_VACUUM:
		// Action
		vacuumSolenoid.Command(ACTIVATE);
		// Transition Conditions
		if (vacuumPressureSwitch.HasVacuum() == true)
		{
			state = IN_POSITION;
#ifdef DEBUG_MODE
			PrintStateTransition(state);
#endif
		}
		break;
	case IN_POSITION:
		// Action
		suctionCup.Command(DEACTIVATE); // so that the board is rested back on the stop
		// Transition Conditions

		break;

	default:
		break;
	}
}
