#ifndef SAVEDDATA_H
#define SAVEDDATA_H
#include <Arduino.h>

class SavedData
{
private:  
    static const int lowerThresholdEepromAddress;
    static const int upperThresholdEepromAddress;
    static const int lowerThresholdHardCodedDefault;
    static const int upperThresholdHardCodedDefault;    
    static int lowerThresholdValue;
    static int upperThresholdValue;

public:
    static int GetLowerThresholdValue();    
    static int GetUpperThresholdValue();
    static void GetSavedDefaultsOrResetIfCorrupt();
    static void SetNewUpperThresholdValue(int value); 
    static void SetNewLowerThresholdValue(int value); 
};

#endif
