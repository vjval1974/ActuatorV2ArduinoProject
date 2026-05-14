#include <stdio.h>
#include "../Data/SavedData.h"
#include "ActuatorStateMachine.h"

static const char* const statesText[] = {
	"STOPPED",
	"ERROR",
	"DRIVING_UP",
	"DRIVING_DOWN",
	"BOARD_SENSED",
	"TUNING_UP",
	"TUNING_DOWN",
	"AT_BOARD",
	"RAISING_CUP",
	"APPLYING_VACUUM",
	"IN_POSITION"
};

void ActuatorStateMachine::PrintStateTransition(ActuatorState state, ActuatorState previousState, const TouchSensor& sensor)
{
#ifdef DEBUG_MODE
	if (state != previousState)
	{
		Serial.print(F("Transitioning State from "));
		Serial.print(statesText[static_cast<uint8_t>(previousState)]);
		Serial.print(F(" to "));
		Serial.print(statesText[static_cast<uint8_t>(state)]);
		Serial.print(F(". Sensor = "));
		Serial.print(sensor.GetFsrPct());
		Serial.println(F("%"));
	}
#endif
#ifdef LOG_MODE
	{
		static unsigned long cycleCount = 0;
		static uint8_t boardSensedCount = 0;
		static uint8_t tuningUpCount = 0;
		static uint8_t tuningDownCount = 0;
		static bool firstLog = true;

		if (firstLog && state == ActuatorState::AT_BOARD)
		{
			Serial.println(F("Cycle\t|Board Sensed\t|Tuning Up\t|Tuning Down\t|Resting Pressure (%)\t|"));
			firstLog = false;
		}
		if (state == ActuatorState::BOARD_SENSED && state != previousState)
			boardSensedCount++;
		if (state == ActuatorState::TUNING_DOWN && state != previousState)
			tuningDownCount++;
		if (state == ActuatorState::TUNING_UP && state != previousState)
			tuningUpCount++;
		if (state == ActuatorState::AT_BOARD && state != previousState)
		{
			char temp[80];
			snprintf(temp, sizeof(temp),
				"%02lu   \t|%02u        \t|%02u       \t|%02u         \t|%03d                  \t|\r\n",
				cycleCount++, boardSensedCount, tuningUpCount, tuningDownCount, sensor.GetFsrPct());
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
#if defined(DEBUG_MODE) || defined(LOG_MODE)
	return state == PressState::SINGLE_PRESS;
#else
	(void)state;
	return false;
#endif
}

ActuatorStateMachine::ActuatorStateMachine()
	: startPushbutton(kStartPushbuttonPin, kPushbuttonDebounceWindow)
	, stopPushbutton(kStopPushbuttonPin, kPushbuttonDebounceWindow)
	, touchSensor(kTouchSensorPin, SavedData::GetLowerThresholdValue(), SavedData::GetUpperThresholdValue())
	, actuatorMotorController()
	, suctionCup(kSuctionCupSolenoidPin, kSuctionCupPositionPin)
	, vacuumPressureSwitch(kVacuumPressureSwitchPin)
	, vacuumSolenoid(kVacuumSolenoidPin)
	, state(ActuatorState::STOPPED)
	, previousState(ActuatorState::STOPPED)
	, atBoardTicks(0)
	, raisingCupTicks(0)
	, applyingVacuumTicks(0)
{
}

void ActuatorStateMachine::ResetActions()
{
	actuatorMotorController.MotorDrive(MotorCommand::MOTOR_STOP);
	vacuumSolenoid.Command(SolenoidCommand::DEACTIVATE);
	suctionCup.Command(SolenoidCommand::DEACTIVATE);
}

void ActuatorStateMachine::ResetStateTimersOnEntry()
{
	if (state != previousState)
	{
		atBoardTicks = 0;
		raisingCupTicks = 0;
		applyingVacuumTicks = 0;
	}
}

void ActuatorStateMachine::Process()
{
	startPushbutton.PollPresses();
	stopPushbutton.PollPresses();
	PressState startButtonState = startPushbutton.IsPress();
	previousState = state;

	switch (state)
	{
	case ActuatorState::STOPPED:
		ResetActions();
		if ((startButtonState == PressState::SINGLE_PRESS &&
			 suctionCup.GetState() == SolenoidState::DEACTIVATED &&
			 suctionCup.GetPosition() == SuctionCupPosition::SUCTION_CUP_LOWERED &&
			 vacuumSolenoid.GetState() == SolenoidState::DEACTIVATED &&
			 vacuumPressureSwitch.HasVacuum() == false &&
			 touchSensor.GetState() == FsrState::BELOW_LOWER_THRESHOLD &&
			 actuatorMotorController.GetMotorState() != MotorState::MOTOR_FAULT) ||
			ShouldTransitionOnPress(startButtonState))
		{
			state = ActuatorState::DRIVING_UP;
		}
		break;

	case ActuatorState::DRIVING_UP:
		actuatorMotorController.MotorDrive(MotorCommand::DRIVE_UP_FAST);
		if (touchSensor.GetState() == FsrState::ABOVE_LOWER_THRESHOLD)
		{
			state = ActuatorState::BOARD_SENSED;
		}
		if (touchSensor.GetState() == FsrState::ABOVE_UPPER_THRESHOLD)
		{
			// safety: overshot the band
			state = ActuatorState::TUNING_DOWN;
		}
		break;

	case ActuatorState::BOARD_SENSED:
		actuatorMotorController.MotorDrive(MotorCommand::MOTOR_STOP);
		if (touchSensor.GetState() == FsrState::ABOVE_LOWER_THRESHOLD || ShouldTransitionOnPress(startButtonState))
		{
			state = ActuatorState::AT_BOARD;
		}
		if (touchSensor.GetState() == FsrState::ABOVE_UPPER_THRESHOLD)
		{
			state = ActuatorState::TUNING_DOWN;
		}
		break;

	case ActuatorState::TUNING_DOWN:
		actuatorMotorController.MotorDrive(MotorCommand::DRIVE_DOWN_SLOW);
		if (touchSensor.GetState() == FsrState::ABOVE_LOWER_THRESHOLD)
		{
			state = ActuatorState::AT_BOARD;
		}
		if (touchSensor.GetState() == FsrState::BELOW_LOWER_THRESHOLD)
		{
			state = ActuatorState::TUNING_UP;
		}
		break;

	case ActuatorState::TUNING_UP:
		actuatorMotorController.MotorDrive(MotorCommand::DRIVE_UP_SLOW);
		if (touchSensor.GetState() == FsrState::ABOVE_LOWER_THRESHOLD)
		{
			state = ActuatorState::AT_BOARD;
		}
		if (touchSensor.GetState() == FsrState::ABOVE_UPPER_THRESHOLD)
		{
			state = ActuatorState::TUNING_DOWN;
		}
		break;

	case ActuatorState::AT_BOARD:
		actuatorMotorController.MotorDrive(MotorCommand::MOTOR_STOP);
		// Real transition wants vacuumPressureSwitch.HasVacuum() == false &&
		// motor stopped — falling back to a tick count until the pressure
		// switch is wired and reliable.
		if (ShouldTransitionOnPress(startButtonState) || atBoardTicks++ >= kAtBoardSettleTicks)
		{
			state = ActuatorState::RAISING_CUP;
		}
		break;

	case ActuatorState::RAISING_CUP:
		suctionCup.Command(SolenoidCommand::ACTIVATE);
		// Real transition wants suctionCup.GetPosition() == SUCTION_CUP_RAISED.
		if (ShouldTransitionOnPress(startButtonState) || raisingCupTicks++ >= kRaisingCupTimeoutTicks)
		{
			state = ActuatorState::APPLYING_VACUUM;
		}
		break;

	case ActuatorState::APPLYING_VACUUM:
		vacuumSolenoid.Command(SolenoidCommand::ACTIVATE);
		// Real transition wants vacuumPressureSwitch.HasVacuum() == true.
		if (ShouldTransitionOnPress(startButtonState) || applyingVacuumTicks++ >= kApplyingVacuumTimeoutTicks)
		{
			state = ActuatorState::IN_POSITION;
		}
		break;

	case ActuatorState::IN_POSITION:
		suctionCup.Command(SolenoidCommand::DEACTIVATE); // rest the board back on the stop
		if (ShouldTransitionOnPress(startButtonState))
		{
			state = ActuatorState::DRIVING_DOWN;
		}
		break;

	case ActuatorState::DRIVING_DOWN:
		actuatorMotorController.MotorDrive(MotorCommand::DRIVE_DOWN_FAST);
		vacuumSolenoid.Command(SolenoidCommand::DEACTIVATE);
		if (ShouldTransitionOnPress(startButtonState))
		{
			state = ActuatorState::STOPPED;
		}
		break;

	case ActuatorState::ERROR:
		ResetActions();
		break;
	}

	ResetStateTimersOnEntry();
	PrintStateTransition(state, previousState, touchSensor);
}
