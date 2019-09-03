#include "TouchSensor.h"
#include "../Helpers/Util.h"
#include <Arduino.h>

// todo: correct this. Take 0 as lower and 1024 as top

TouchSensor::TouchSensor(int inputPin, int lowerThresholdPct, int upperThresholdPct)
{
    _inputPin = inputPin;
    _lowerThresholdPct = lowerThresholdPct;
    _upperThresholdPct = upperThresholdPct;
    pinMode(inputPin, INPUT);
}
int TouchSensor::GetFsrPct()
{
    int retVal = analogRead(_inputPin);
    return (int)map(retVal, 0, 1023, 0, 100);
}

FsrState TouchSensor::GetState(void)
{
    // check analag input value
    // filter
    int fsrValue = GetFsrPct();
#ifdef DEBUG_MODE
    Serial.print("Touch Sensor value: ");
    Serial.println(fsrValue);
#endif
    if (fsrValue <= _lowerThresholdPct)
        return BELOW_LOWER_THRESHOLD;

    else if (fsrValue <= _upperThresholdPct)
        return ABOVE_LOWER_THRESHOLD;

    else
        return ABOVE_UPPER_THRESHOLD;
}