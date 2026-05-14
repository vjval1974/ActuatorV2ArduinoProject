#ifndef PRESSURESWITCHSTATE_H
#define PRESSURESWITCHSTATE_H

#include <stdint.h>

enum class PressureSwitchState : uint8_t {
    HAS_VACUUM_PRESSURE,
    HAS_NO_VACUUM_PRESSURE
};

#endif
