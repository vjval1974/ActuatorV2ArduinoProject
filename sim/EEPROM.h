// EEPROM shim for the simulator. Backs a 1024-byte array (matching the
// ATmega32U4's onboard EEPROM size). update() only writes when the value
// has changed, matching the real Arduino EEPROM library semantics.

#ifndef SIM_EEPROM_H
#define SIM_EEPROM_H

#include <stdint.h>

constexpr int kSimEepromSize = 1024;

class SimEepromClass
{
public:
    uint8_t data[kSimEepromSize];

    SimEepromClass()
    {
        // Match a freshly-erased AVR EEPROM where every byte reads 0xFF.
        for (int i = 0; i < kSimEepromSize; ++i)
            data[i] = 0xFF;
    }

    uint8_t read(int addr) const
    {
        if (addr < 0 || addr >= kSimEepromSize) return 0;
        return data[addr];
    }

    void write(int addr, uint8_t value)
    {
        if (addr < 0 || addr >= kSimEepromSize) return;
        data[addr] = value;
    }

    void update(int addr, uint8_t value)
    {
        if (addr < 0 || addr >= kSimEepromSize) return;
        if (data[addr] != value)
            data[addr] = value;
    }
};

extern SimEepromClass EEPROM;

#endif
