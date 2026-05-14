#ifndef TOUCHSENSOR_H
#define TOUCHSENSOR_H

#include "FsrState.h"

class TouchSensor
{
private:
    int _inputPin;
    int _lowerThresholdPct;
    int _upperThresholdPct;

public:
    TouchSensor(int inputPin, int lowerThresholdPct, int upperThresholdPct);
    TouchSensor(const TouchSensor&) = delete;
    TouchSensor& operator=(const TouchSensor&) = delete;
    FsrState GetState() const;
    int GetFsrPct() const;
};

#endif
