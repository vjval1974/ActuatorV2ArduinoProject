#ifndef FSRPUSHBUTTON_H
#define FSRPUSHBUTTON_H

#include <Arduino.h>
#define ARRAY_LENGTH 200

enum PressState
{
    SINGLE_PRESS,
    DOUBLE_PRESS,
    NOT_PRESSED
};

class FsrPushbutton
{
private:
    uint8_t valueArray[ARRAY_LENGTH];
    uint8_t _numSpacesForTimeout;
    bool _hasOneRelease;
    uint8_t _oneReleaseCount;
    bool _hasTwoReleases;
    int _analogInput;
    PressState currentPressState = NOT_PRESSED;

public:
    FsrPushbutton(int analogInput, uint8_t numSpacesForTimeout);
    void PollPresses();
    PressState IsPress();
    bool IsDownPress();
    bool IsRelease();
};

#endif