#ifndef SOLENOIDCOMMAND_H
#define SOLENOIDCOMMAND_H

#include <stdint.h>

enum class SolenoidCommand : uint8_t {
    ACTIVATE,
    DEACTIVATE
};

#endif
