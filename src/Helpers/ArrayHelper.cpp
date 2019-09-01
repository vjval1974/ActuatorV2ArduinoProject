#include <Arduino.h>
#include "ArrayHelper.h"

void ArrayHelper::PrintArray(uint8_t *array, uint8_t len)
{
	Serial.println("========================");
	for (int i = 0; i < len; i++)
	{
		Serial.print(i);
		Serial.print(": ");
		Serial.print(array[i]);
		Serial.print(", ");
	}
	Serial.println("========================");
}

void ArrayHelper::ClearArray(uint8_t *array, uint8_t len)
{
	for (int i = 0; i < len; i++)
		array[i] = 0;
}