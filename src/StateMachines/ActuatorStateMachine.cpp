#include <stdio.h>

#include "ActuatorStateMachine.h"

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

void ActuatorStateMachine::PrintStateTransition(ActuatorState state, ActuatorState previousState)
{
#ifdef DEBUG_MODE
	if (state != previousState)
	{
		Serial.print("Transitioning State from ");
		Serial.print(statesText[previousState]);
		Serial.print(" to ");
		Serial.println(statesText[state]);
	}
#endif
}

bool ActuatorStateMachine::ShouldTransitionOnPress(PressState state)
{
	bool retval = false;
#ifdef DEBUG_MODE
	retval = (state == SINGLE_PRESS);
#endif
	return retval;
}

ActuatorStateMachine::ActuatorStateMachine()
{
	startPushbutton = FsrPushbutton(A1, 5);
	stopPushbutton = FsrPushbutton(A2, 5);
	touchSensor = TouchSensor(A1, 20, 80);
	actuatorMotorController = MotorController(PIN1, PIN2, PIN3, PIN4, PIN5);
	suctionCup = SuctionCup(PIN6, PIN7);
	vacuumPressureSwitch = VacuumPressureSwitch(PINB0);
	vacuumSolenoid = VacuumSolenoid(PINB1);
	state = STOPPED;
	previousState = STOPPED;
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
	previousState = state;

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
			 actuatorMotorController.GetMotorState() != MOTOR_FAULT) ||
			ShouldTransitionOnPress(startButtonState))
		{
			state = DRIVING_UP;
		}
		break;
	case DRIVING_UP:
		//Action
		actuatorMotorController.MotorDrive(DRIVE_UP_FAST);

		//Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD || ShouldTransitionOnPress(startButtonState))
		{
			state = BOARD_SENSED;
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD || ShouldTransitionOnPress(startButtonState))
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
		}
		break;

	case BOARD_SENSED:
		// Action
		actuatorMotorController.MotorDrive(MOTOR_STOP);

		// Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD || ShouldTransitionOnPress(startButtonState))
		{
			state = AT_BOARD;
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
		}
		break;
	case TUNING_DOWN:
		// Action
		actuatorMotorController.MotorDrive(DRIVE_DOWN_SLOW);
		// Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
		}
		if (touchSensor.GetState() == BELOW_LOWER_THRESHOLD)
		{
			state = TUNING_UP;
		}
		break;
	case TUNING_UP:
		// Action
		actuatorMotorController.MotorDrive(DRIVE_UP_SLOW);

		// Transition Conditions
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD)
		{
			state = AT_BOARD;
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD)
		{
			// shouldnt get here but put here for safety
			state = TUNING_DOWN;
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
		}

		break;
	case RAISING_CUP:
		// Action
		suctionCup.Command(ACTIVATE);
		// Transition Conditions
		if (suctionCup.GetPosition() == SUCTION_CUP_RAISED)
		{
			state = APPLYING_VACUUM;
		}
		break;
	case APPLYING_VACUUM:
		// Action
		vacuumSolenoid.Command(ACTIVATE);
		// Transition Conditions
		if (vacuumPressureSwitch.HasVacuum() == true)
		{
			state = IN_POSITION;
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
	PrintStateTransition(state, previousState);
}
