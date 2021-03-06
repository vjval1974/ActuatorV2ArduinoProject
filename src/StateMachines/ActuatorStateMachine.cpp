#include <stdio.h>
#include "../Data/SavedData.h"
#include "ActuatorStateMachine.h"

String statesText[11] =
	{"STOPPED",
	 "ERROR",
	 "DRIVING_UP",
	 "DRIVING_DOWN",
	 "BOARD_SENSED",
	 "TUNING_UP",
	 "TUNING_DOWN",
	 "AT_BOARD",
	 "RAISING_CUP",
	 "APPLYING_VACUUM",
	 "IN_POSITION"};

void ActuatorStateMachine::PrintStateTransition(ActuatorState state, ActuatorState previousState, TouchSensor sensor)
{
#ifdef DEBUG_MODE
	if (state != previousState)
	{
		Serial.print("Transitioning State from ");
		Serial.print(statesText[previousState]);
		Serial.print(" to ");
		Serial.print(statesText[state]);
		Serial.print(". Sensor = ");
		Serial.print(sensor.GetFsrPct());
		Serial.println("%");
	}
#endif
#ifdef LOG_MODE
	{
		static unsigned long cycleCount = 0;
		static uint8_t boardSensedCount = 0;
		static uint8_t tuningUpCount = 0;
		static uint8_t tuningDownCount = 0;
		static int pct;
		static bool firstLog = true;

		if (firstLog && state == AT_BOARD)
		{
			Serial.println("Cycle\t|Board Sensed\t|Tuning Up\t|Tuning Down\t|Resting Pressure (%)\t|");
			firstLog = false;
		}
		if (state == BOARD_SENSED && state != previousState)
			boardSensedCount++;
		if (state == TUNING_DOWN && state != previousState)
			tuningDownCount++;
		if (state == TUNING_UP && state != previousState)
			tuningUpCount++;
		if (state == AT_BOARD && state != previousState)
		{

			pct = sensor.GetFsrPct();
			static char temp[200];
			sprintf(temp, "%02lu   \t|%02u        \t|%02u       \t|%02u         \t|%03d                  \t|\r\n",
					cycleCount++, boardSensedCount, tuningUpCount, tuningDownCount, pct);
			Serial.print(temp);
			Serial.flush();
			boardSensedCount = 0;
			tuningUpCount = 0;
			tuningDownCount = 0;
		}
		


	
	}
#endif


}

bool ActuatorStateMachine::ShouldTransitionOnPress(PressState state)
{
	bool retval = false;
#ifdef DEBUG_MODE
	retval = (state == SINGLE_PRESS);
#endif
#ifdef LOG_MODE
	retval = (state == SINGLE_PRESS);
#endif
	return retval;
}

ActuatorStateMachine::ActuatorStateMachine()
{
	startPushbutton = FsrPushbutton(A0, 5);
	stopPushbutton = FsrPushbutton(A1, 5);
	touchSensor = TouchSensor(A2,  SavedData::GetLowerThresholdValue(), SavedData::GetUpperThresholdValue());
	actuatorMotorController = MotorController(); // default pins set in constructor
	suctionCup = SuctionCup(9, 14);
	vacuumPressureSwitch = VacuumPressureSwitch(10);
	vacuumSolenoid = VacuumSolenoid(8);
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
		if (touchSensor.GetState() == ABOVE_LOWER_THRESHOLD) // || ShouldTransitionOnPress(startButtonState))
		{
			state = BOARD_SENSED;
		}
		if (touchSensor.GetState() == ABOVE_UPPER_THRESHOLD) // || ShouldTransitionOnPress(startButtonState))
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
		// if (vacuumPressureSwitch.HasVacuum() == false && actuatorMotorController.GetMotorState() == MOTOR_STOPPED)
		// {
		// 	state = RAISING_CUP;
		// }
		// if (ShouldTransitionOnPress(startButtonState))
		// {
		// 	state = DRIVING_DOWN;
		// }
		// if (ShouldTransitionOnPress(stopButtonState))
		// {
		// 	state = STOPPED;
		// }
		//Serial.println(touchSensor.GetFsrPct());
		static long cnt = 0;
		if (ShouldTransitionOnPress(startButtonState))
			state = RAISING_CUP;
		if (cnt++ >= 20)
		{
			state = RAISING_CUP;
			
			cnt = 0;
		}
		break;
	case RAISING_CUP:
		// Action
		suctionCup.Command(ACTIVATE);
		// Transition Conditions 
		// Todo: add some sort of delay here as we dont have position switch
		// if (suctionCup.GetPosition() == SUCTION_CUP_RAISED)
		// {
		// 	state = APPLYING_VACUUM;
		// }
		if (ShouldTransitionOnPress(startButtonState))
			state = APPLYING_VACUUM;
			if (cnt++ >= 400)
		{
			state = APPLYING_VACUUM;
			
			cnt = 0;
		}
		break;
	case APPLYING_VACUUM:
		// Action
		vacuumSolenoid.Command(ACTIVATE);
		// Transition Conditions
		// if (vacuumPressureSwitch.HasVacuum() == true)
		// {
		// 	state = IN_POSITION;
		// }
		if (ShouldTransitionOnPress(startButtonState))
			state = IN_POSITION;
			if (cnt++ >= 400)
		{
			state = IN_POSITION;
			
			cnt = 0;
		}	
		break;
	case IN_POSITION:
		// Action
		suctionCup.Command(DEACTIVATE); // so that the board is rested back on the stop
		// Transition Conditions
if (ShouldTransitionOnPress(startButtonState))
			state = DRIVING_DOWN;
		break;
	case DRIVING_DOWN:
	{
		actuatorMotorController.MotorDrive(DRIVE_DOWN_FAST);
		vacuumSolenoid.Command(DEACTIVATE);
		if (ShouldTransitionOnPress(startButtonState))
		{
			state = STOPPED;
		}
		// static long cnt2 = 0;
		// if (cnt2++ >= 20)
		// {
		// 	state = DRIVING_UP;
		// 	cnt2 = 0;
		// }
		break;
	}

	default:
		break;
	}
	PrintStateTransition(state, previousState, touchSensor);
}
