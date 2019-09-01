#include <stdint.h>
#include <Arduino.h>
#include "FsrPushbutton.h"
#include "../Helpers/ArrayHelper.h"

FsrPushbutton::FsrPushbutton(int analogInput, uint8_t numSpacesForTimeout)
{
    _analogInput = analogInput;
    _numSpacesForTimeout = numSpacesForTimeout;
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
    static bool hasOneRelease = false;
    static uint8_t oneReleaseCount = 0;
    static bool hasTwoReleases = false;
    PressState retval = NOT_PRESSED;
    if (!hasOneRelease)
    {
        hasOneRelease = IsRelease();
    }

    if (hasOneRelease)
    {
        oneReleaseCount++;

        if (oneReleaseCount >= 2 && hasTwoReleases == false)
        {
            hasTwoReleases = IsRelease();
        }
        if (oneReleaseCount > _numSpacesForTimeout)
        {
            retval = hasTwoReleases ? DOUBLE_PRESS : SINGLE_PRESS;
            hasOneRelease = false;
            hasTwoReleases = false;
            oneReleaseCount = 0;
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