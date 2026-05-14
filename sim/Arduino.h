// Native Arduino shim for the actuator simulator.
//
// Lets the existing PlatformIO/Arduino-targeted firmware compile and run
// under plain g++ on a laptop. Provides the small subset of Arduino-core
// APIs that the firmware actually touches:
//   pinMode / digitalRead / digitalWrite / analogRead / analogWrite
//   millis / micros / delay / delayMicroseconds
//   Serial (writes captured into a buffer, flushed per tick)
//   F() / __FlashStringHelper (no-ops on native)
//   map()
//
// Pin state is held in two global arrays (digital / analog) keyed by pin
// number. The physics layer reads motor/solenoid pins and writes sensor
// pins, so the firmware can never tell it is being driven by a simulator.

#ifndef SIM_ARDUINO_SHIM_H
#define SIM_ARDUINO_SHIM_H

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <cstring>

#define HIGH 1
#define LOW 0

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

// Pro Micro analog pin aliases. Numeric values are arbitrary as long as
// they're distinct from the digital pin numbers actually used by the
// firmware (2-14).
#define A0 18
#define A1 19
#define A2 20
#define A3 21
#define A4 22
#define A5 23
#define A6 24

constexpr int kSimMaxPins = 32;

class SimPinState
{
public:
    static uint8_t digitalLevels[kSimMaxPins];
    static int analogLevels[kSimMaxPins];
    static uint8_t pinModes[kSimMaxPins];
    static unsigned long bootMillis;
    static std::string serialBuffer;
};

void pinMode(uint8_t pin, uint8_t mode);
void digitalWrite(uint8_t pin, uint8_t value);
int digitalRead(uint8_t pin);
int analogRead(uint8_t pin);
void analogWrite(uint8_t pin, int value);
void delay(unsigned long ms);
void delayMicroseconds(unsigned long us);
unsigned long millis();
unsigned long micros();

long map(long x, long in_min, long in_max, long out_min, long out_max);

// __FlashStringHelper is the Arduino type returned by F("..."). On AVR it
// lives in PROGMEM; here it's just a regular const char*.
class __FlashStringHelper;
#define F(string_literal) ((const __FlashStringHelper*)(string_literal))

class SimSerial
{
public:
    void begin(unsigned long baud);
    void print(const char* s);
    void print(const __FlashStringHelper* s);
    void print(int v);
    void print(unsigned int v);
    void print(long v);
    void print(unsigned long v);
    void print(double v);
    void println();
    void println(const char* s);
    void println(const __FlashStringHelper* s);
    void println(int v);
    void println(unsigned int v);
    void println(long v);
    void println(unsigned long v);
    void println(double v);
    void flush();
};

extern SimSerial Serial;

#endif
