#ifndef FSRSTATE_H
#define FSRSTATE_H

#include <stdint.h>

enum class FsrState : uint8_t {
    ABOVE_LOWER_THRESHOLD,
    BELOW_LOWER_THRESHOLD,
    ABOVE_UPPER_THRESHOLD
};

#endif
