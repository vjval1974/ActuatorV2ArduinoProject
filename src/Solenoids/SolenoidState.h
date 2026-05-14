#ifndef SOLENOIDSTATE_H
#define SOLENOIDSTATE_H

#include <stdint.h>

enum class SolenoidState : uint8_t {
    ACTIVATED,
    DEACTIVATED
};

#endif
