#include <stdint.h>
#include <Arduino.h>
#include "FsrPushbutton.h"
#include "../Helpers/ArrayHelper.h"

FsrPushbutton::FsrPushbutton(int analogInput, uint8_t numSpacesForTimeout)
{
    _analogInput = analogInput;
    _numSpacesForTimeout = numSpacesForTimeout;
    _hasOneRelease = false;
    _oneReleaseCount = 0;
    _hasTwoReleases = false;
    pinMode(analogInput, INPUT);
    ArrayHelper::ClearArray(valueArray, ARRAY_LENGTH);
}

void FsrPushbutton::PollPresses()
{
    // shift array right then save new value
    for (int i = ARRAY_LENGTH - 1; i > 0; i--)
    {
        valueArray[i] = valueArray[i - 1];
    }
    uint8_t analogValue = analogRead(_analogInput);
    valueArray[0] = analogValue;
}

PressState FsrPushbutton::IsPress()
{
    PressState retval = NOT_PRESSED;
    if (!_hasOneRelease)
    {
        _hasOneRelease = IsRelease();
    }

    if (_hasOneRelease)
    {
        _oneReleaseCount++;

        if (_oneReleaseCount >= 2 && _hasTwoReleases == false)
        {
            _hasTwoReleases = IsRelease();
        }
        if (_oneReleaseCount > _numSpacesForTimeout)
        {
            retval = _hasTwoReleases ? DOUBLE_PRESS : SINGLE_PRESS;
            _hasOneRelease = false;
            _hasTwoReleases = false;
            _oneReleaseCount = 0;
        }
    }

    return retval;
}

bool FsrPushbutton::IsDownPress()
{
    return (valueArray[0] > (uint8_t)0 && valueArray[1] == 0);
}

bool FsrPushbutton::IsRelease()
{
    return (valueArray[1] > (uint8_t)1 && valueArray[0] == 0);
}