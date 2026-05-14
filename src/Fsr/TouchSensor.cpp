#include "TouchSensor.h"
#include <Arduino.h>

// todo: correct this. Take 0 as lower and 1024 as top

TouchSensor::TouchSensor(int inputPin, int lowerThresholdPct, int upperThresholdPct)
{
    _inputPin = inputPin;
    _lowerThresholdPct = lowerThresholdPct;
    _upperThresholdPct = upperThresholdPct;
    pinMode(inputPin, INPUT);
}
int TouchSensor::GetFsrPct() const
{
    int retVal = analogRead(_inputPin);
    return (int)map(retVal, 0, 1023, 0, 100);
}

FsrState TouchSensor::GetState() const
{
    int fsrValue = GetFsrPct();
    if (fsrValue <= _lowerThresholdPct)
        return FsrState::BELOW_LOWER_THRESHOLD;
    else if (fsrValue <= _upperThresholdPct)
        return FsrState::ABOVE_LOWER_THRESHOLD;
    else
        return FsrState::ABOVE_UPPER_THRESHOLD;
}