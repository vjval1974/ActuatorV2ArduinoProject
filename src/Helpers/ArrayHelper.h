#ifndef ARRAYHELPERS_H
#define ARRAYHELPERS_H
#include <Arduino.h>

class ArrayHelper
{
public:
    static void PrintArray(uint8_t *array, uint8_t len);
    static void ClearArray(uint8_t *array, uint8_t len);
};

#endif