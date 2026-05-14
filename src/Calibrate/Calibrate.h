#ifndef CALIBRATE_H
#define CALIBRATE_H

#include "../Pushbuttons/FsrPushbutton.h"
#include "../Motor/Motor.h"
#include "../Fsr/TouchSensor.h"

enum class CalibrationState : uint8_t
{
    STOPPED,
    DRIVE_DOWN_FAST,
    DRIVING_DOWN_FAST,
    DRIVING_DOWN_10S_ELAPSED,
    DRIVING_UP_SLOW,
    PUSHBUTTON_PRESSED,
    DRIVING_DOWN_SLOW,
    SENSOR_READINGS_HAVE_DROPPED,
    SENSOR_READINGS_STABLE,
    TUNING_COMPLETE,
    STORING_VALUES
};

class Calibrate
{
private:
    static constexpr uint8_t kDefaultLowerThreshold = 15;
    static constexpr uint8_t kDefaultUpperThreshold = 25;

    CalibrationState state;
    CalibrationState previousState;
    MotorController& _motor;
    TouchSensor& _touchSensor;
    FsrPushbutton& _startPushbutton;
    FsrPushbutton& _stopPushbutton;

public:
    Calibrate(MotorController& motor,
              TouchSensor& touchSensor,
              FsrPushbutton& startPushbutton,
              FsrPushbutton& stopPushbutton);
    void Go();
};

#endif
