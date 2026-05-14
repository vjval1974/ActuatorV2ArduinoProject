#ifndef ACTUATORSTATEMACHINE_H
#define ACTUATORSTATEMACHINE_H

#include "../VacuumPressureSwitch/VacuumPressureSwitch.h"
#include "../Solenoids/SuctionCupSolenoid.h"
#include "../Solenoids/VacuumSolenoid.h"
#include "../Pushbuttons/FsrPushbutton.h"
#include "../Motor/Motor.h"
#include "../Fsr/TouchSensor.h"


enum class ActuatorState : uint8_t
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
};

class ActuatorStateMachine {
private:
	// Pro Micro silkscreen pins — see main.cpp header comment for the full map.
	static constexpr uint8_t kStartPushbuttonPin = A0;
	static constexpr uint8_t kStopPushbuttonPin = A1;
	static constexpr uint8_t kTouchSensorPin = A2;
	static constexpr uint8_t kSuctionCupSolenoidPin = 9;
	static constexpr uint8_t kSuctionCupPositionPin = 14;
	static constexpr uint8_t kVacuumPressureSwitchPin = 10;
	static constexpr uint8_t kVacuumSolenoidPin = 8;
	static constexpr uint8_t kPushbuttonDebounceWindow = 5;

	// Tick-based fallback timeouts used while real position/vacuum feedback is
	// not wired up. Process() runs every 20 ms, so 20 ticks ≈ 0.4 s and
	// 400 ticks ≈ 8 s. Remove these once the corresponding sensors are wired.
	static constexpr long kAtBoardSettleTicks = 20;
	static constexpr long kRaisingCupTimeoutTicks = 400;
	static constexpr long kApplyingVacuumTimeoutTicks = 400;

	FsrPushbutton startPushbutton;
	FsrPushbutton stopPushbutton;
	TouchSensor touchSensor;
	MotorController actuatorMotorController;
	SuctionCup suctionCup;
	VacuumPressureSwitch vacuumPressureSwitch;
	VacuumSolenoid vacuumSolenoid;
	ActuatorState state;
	ActuatorState previousState;
	long atBoardTicks;
	long raisingCupTicks;
	long applyingVacuumTicks;

	void ResetActions();
	void ResetStateTimersOnEntry();
	static bool ShouldTransitionOnPress(PressState state);
	static void PrintStateTransition(ActuatorState state, ActuatorState previousState, const TouchSensor& sensor);

public:
	ActuatorStateMachine();
	ActuatorStateMachine(const ActuatorStateMachine&) = delete;
	ActuatorStateMachine& operator=(const ActuatorStateMachine&) = delete;
	void Process();
};

#endif
