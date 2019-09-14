#include "Calibrate.h"
#include "../Data/SavedData.h"

String CalibrationsStatesText[11] =
    {
        "STOPPED",
        "DRIVE_DOWN_FAST",
        "DRIVING_DOWN_FAST",
        "DRIVING_DOWN_10S_ELAPSED",
        "DRIVING_UP_SLOW",
        "PUSHBUTTON_PRESSED",
        "DRIVING_DOWN_SLOW",
        "SENSOR_READINGS_HAVE_DROPPED",
        "SENSOR_READINGS_STABLE",
        "TUNING_COMPLETE",
        "STORING_VALUES"};

Calibrate::Calibrate()
{
    startPushbutton = FsrPushbutton(A0, 5);
    stopPushbutton = FsrPushbutton(A1, 5);
    touchSensor = TouchSensor(A2, SavedData::GetLowerThresholdValue(), SavedData::GetUpperThresholdValue());
    actuatorMotorController = MotorController(); // default pins set in constructor
    state = STOPPED;
    previousState = STOPPED;
}

static bool IsValueInWindow(int value, int lower, int upper)
{
    return (value >= lower) || (value <= upper);
}
void Calibrate::Go()
{

    /*
1. wait for pushbutton to start
2. drive down for 10s
3. drive up slow
4. wait for pushbutton to indicate that the board has lifted
5. record FSR value. 
6. drive down slow.
7. when the value drops by x percent move to next step
8. look for y values that are within +- 3 of each other. 
9. Stop. 
10. record FSR value
11. calculate upper and lower values to save to eeprom

Example: 
upper = 50 
lower = 10
We know that this is 0.2mm (based on the air gap). Therefore 40% = 0.2mm (I know 
the transfer function isnt linear, but we can incorporate an equation later). 

So if we give the lower some room to move, say add 10%. lower = 20
And we give a 20% buffer from the top value, so upper = 30;

The best case scenario is if the actuator stops at 30%. This will allow the head to 
squash 20% before hitting the stop. 0.1mm 
The worst case is if we stop at the lower threshold at 20% which equates to 0.15mm. 
*/
    const uint8_t upperBuffer = 20; // %
    const uint8_t lowerBugger = 10; // %
    static uint8_t cntOfStabiliseStepIterations = 0;
    startPushbutton.PollPresses();
    stopPushbutton.PollPresses();
    PressState startButtonState = startPushbutton.IsPress();
    //PressState stopButtonState = stopPushbutton.IsPress();
    previousState = state;
    unsigned long driveDownTimerStart = 0;
    uint8_t readingWhenBoardLifted = 0;
    uint8_t readingWhenLoweredAndReadingSettled = 0;

    switch (state)
    {
    case STOPPED:
    {
        //Action
        actuatorMotorController.MotorDrive(MOTOR_STOP);

        //Transition Conditions
        if (startButtonState == SINGLE_PRESS)
        {
            state = DRIVE_DOWN_FAST;
        }
        break;
    }
    case DRIVE_DOWN_FAST:
    {
        //Action
        actuatorMotorController.MotorDrive(MotorCommand::DRIVE_DOWN_FAST);
        driveDownTimerStart = millis();
        state = DRIVING_DOWN_FAST;
        break;
    }
    case DRIVING_DOWN_FAST:
    {
        //Action

        //Transition Conditions
        if (millis() - driveDownTimerStart > 10000)
        {
            state = DRIVING_DOWN_10S_ELAPSED;
        }
        break;
    }
    case DRIVING_DOWN_10S_ELAPSED:
    {
        //Action
        actuatorMotorController.MotorDrive(MotorCommand::MOTOR_STOP);

        state = DRIVING_UP_SLOW;
        break;
    }

    case DRIVING_UP_SLOW:
    {
        //Action
        actuatorMotorController.MotorDrive(MotorCommand::DRIVE_UP_SLOW);

        //Transition Conditions
        if (startButtonState == PressState::SINGLE_PRESS)
        {
            state = PUSHBUTTON_PRESSED;
        }
        break;
    }

    case PUSHBUTTON_PRESSED:
    {
        //Action
        actuatorMotorController.MotorDrive(MotorCommand::MOTOR_STOP);
        readingWhenBoardLifted = touchSensor.GetFsrPct();
        //Transition Conditions
        state = DRIVING_DOWN_SLOW;

        break;
    }

    case DRIVING_DOWN_SLOW:
    {
        //Action
        actuatorMotorController.MotorDrive(MotorCommand::DRIVE_DOWN_SLOW);

        //Transition Conditions
        if (touchSensor.GetFsrPct() <= (readingWhenBoardLifted - upperBuffer))
        {
            state = SENSOR_READINGS_HAVE_DROPPED;
            cntOfStabiliseStepIterations = 0;
        }

        break;
    }

    case SENSOR_READINGS_HAVE_DROPPED:
    {
        //Action
        // We want to keep driving here until we get stable values.
        /* 
            1. Record an initial reading (first)
            2. Break
            3. Record a second readong 
                3a. If first - second <= |3| 
                    True: record a third
                    False: save second reading into first reading and go to 3
            4. Record a third reading 
                4a. If third - second <= |3|
                    True: go to 5 
                    False: clear second, save 3rd to first and go to 3
            5. change to next state (3 stable readings achieved)

        */
        static uint8_t firstReading = touchSensor.GetFsrPct();
        static uint8_t secondReading = 0;
        static uint8_t thirdReading = 0;
        const int window = 3;
        if (cntOfStabiliseStepIterations == 1)
        {
            secondReading = touchSensor.GetFsrPct();
            if (cntOfStabiliseStepIterations == 2 &&
                IsValueInWindow(secondReading, firstReading - window, firstReading + window))                                )
                {
                    thirdReading = touchSensor.GetFstPct();
                }
            else
            {
                {
                }
            }
        }

        //Transition Conditions

        cntOfStabiliseStepIterations++;

        break;
    }

    case SENSOR_READINGS_STABLE:
    {

        break;
    }

    case TUNING_COMPLETE:
    {

        break;
    }

    case STORING_VALUES:
    {

        break;
    }
    }
}
