#ifndef FSRPUSHBUTTON_H
#define FSRPUSHBUTTON_H

#include <Arduino.h>

enum class PressState : uint8_t {
    SINGLE_PRESS,
    DOUBLE_PRESS,
    NOT_PRESSED
};

class FsrPushbutton
{
private:
    static constexpr uint8_t kArrayLength = 200;
    static constexpr uint8_t kPressDetectionWindow = 5;
    static constexpr uint8_t kPressDetectionThreshold = 10;

    uint8_t valueArray[kArrayLength];
    uint8_t _numSpacesForTimeout;
    bool _hasOneRelease;
    uint8_t _oneReleaseCount;
    bool _hasTwoReleases;
    int _analogInput;

public:
    FsrPushbutton(int analogInput, uint8_t numSpacesForTimeout);
    FsrPushbutton(const FsrPushbutton&) = delete;
    FsrPushbutton& operator=(const FsrPushbutton&) = delete;
    void PollPresses();
    PressState IsPress();
    bool IsDownPress() const;
    bool IsRelease() const;
};

#endif
