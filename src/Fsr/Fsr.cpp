#include "FsrState.h"
#include "Fsr.h"
#include <stdint.h>
#include <Arduino.h>

// todo: correct this. Take 0 as lower and 1024 as top
#define LOWER_THRESHOLD 20
#define UPPER_THRESHOLD 40
#define FSR_PIN A0;

long map(long x, long in_min, long in_max, long out_min, long out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static int GetFsrPct()
{
    int retVal = analogRead(A0);
    return (int)map(retVal, 0, 1023, 0, 100);
}

FsrState GetFsrState(void)
{
    // check analag input value
    // filter 
     uint16_t fsrValue = GetFsrPct();

    if (fsrValue <= LOWER_THRESHOLD )
        return BELOW_LOWER_THRESHOLD;   

    else if (fsrValue <= UPPER_THRESHOLD)
        return ABOVE_LOWER_THRESHOLD;

    else 
        return ABOVE_UPPER_THRESHOLD;

}