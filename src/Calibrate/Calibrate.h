#ifndef CALIBRATE_H
#define CALIBRATE_H

#include "../Pushbuttons/FsrPushbutton.h"
#include "../Motor/Motor.h"
#include "../Fsr/TouchSensor.h"

typedef enum
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

} CalibrationState;

class Calibrate
{
private:
    const int _defaultLowerThreshold = 15;
    const int _defaultUpperThreshold = 25;
    CalibrationState state;
    CalibrationState previousState;
    FsrPushbutton startPushbutton = FsrPushbutton(A0, 5);
	FsrPushbutton stopPushbutton = FsrPushbutton(A1, 5);
	TouchSensor touchSensor = TouchSensor(A2, 20, 80);
	MotorController actuatorMotorController = MotorController(PIN1, PIN2, PIN3, PIN4, PIN5);
	
public:
    void Go();
};

#endif