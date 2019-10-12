#include <EEPROM.h>
#include "SavedData.h"

const int SavedData::lowerThresholdEepromAddress = 0;
const int SavedData::upperThresholdEepromAddress = 1;
const int SavedData::lowerThresholdHardCodedDefault = 10;
const int SavedData::upperThresholdHardCodedDefault = 20;    
int SavedData::lowerThresholdValue;
int SavedData::upperThresholdValue;


int SavedData::GetLowerThresholdValue()
{
    lowerThresholdValue = EEPROM.read(lowerThresholdEepromAddress);
    if (lowerThresholdValue <= 0 || lowerThresholdValue >= 75)
    {
        lowerThresholdValue = lowerThresholdHardCodedDefault;
        EEPROM.update(lowerThresholdEepromAddress, lowerThresholdValue);
    }

    return lowerThresholdValue;
}

int SavedData::GetUpperThresholdValue()
{
    upperThresholdValue = EEPROM.read(upperThresholdEepromAddress);
    if (upperThresholdValue <= 0 || upperThresholdValue >= 75 || upperThresholdValue <= lowerThresholdValue)
    {
        upperThresholdValue = upperThresholdHardCodedDefault;
        EEPROM.update(upperThresholdEepromAddress, upperThresholdValue);
    }
    return upperThresholdValue;
}
