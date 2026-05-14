#include "Arduino.h"
#include "EEPROM.h"
#include <chrono>
#include <thread>
#include <cstdio>

SimEepromClass EEPROM;

uint8_t SimPinState::digitalLevels[kSimMaxPins] = {0};
int SimPinState::analogLevels[kSimMaxPins] = {0};
uint8_t SimPinState::pinModes[kSimMaxPins] = {0};
unsigned long SimPinState::bootMillis = 0;
std::string SimPinState::serialBuffer;

static unsigned long real_now_ms()
{
    using namespace std::chrono;
    return (unsigned long)duration_cast<milliseconds>(
        steady_clock::now().time_since_epoch()).count();
}

void pinMode(uint8_t pin, uint8_t mode)
{
    if (pin >= kSimMaxPins) return;
    SimPinState::pinModes[pin] = mode;
    if (mode == INPUT_PULLUP)
        SimPinState::digitalLevels[pin] = HIGH;
}

void digitalWrite(uint8_t pin, uint8_t value)
{
    if (pin >= kSimMaxPins) return;
    SimPinState::digitalLevels[pin] = value ? HIGH : LOW;
}

int digitalRead(uint8_t pin)
{
    if (pin >= kSimMaxPins) return 0;
    return SimPinState::digitalLevels[pin];
}

int analogRead(uint8_t pin)
{
    if (pin >= kSimMaxPins) return 0;
    return SimPinState::analogLevels[pin];
}

void analogWrite(uint8_t pin, int value)
{
    if (pin >= kSimMaxPins) return;
    SimPinState::analogLevels[pin] = value;
}

void delay(unsigned long ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

void delayMicroseconds(unsigned long us)
{
    std::this_thread::sleep_for(std::chrono::microseconds(us));
}

unsigned long millis()
{
    if (SimPinState::bootMillis == 0)
        SimPinState::bootMillis = real_now_ms();
    return real_now_ms() - SimPinState::bootMillis;
}

unsigned long micros()
{
    return millis() * 1000UL;
}

long map(long x, long in_min, long in_max, long out_min, long out_max)
{
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// ----- Serial -----

SimSerial Serial;

void SimSerial::begin(unsigned long) {}

void SimSerial::print(const char* s)
{
    if (s) SimPinState::serialBuffer.append(s);
}
void SimSerial::print(const __FlashStringHelper* s) { print((const char*)s); }
void SimSerial::print(int v)           { char b[16]; snprintf(b, sizeof(b), "%d",  v); SimPinState::serialBuffer.append(b); }
void SimSerial::print(unsigned int v)  { char b[16]; snprintf(b, sizeof(b), "%u",  v); SimPinState::serialBuffer.append(b); }
void SimSerial::print(long v)          { char b[24]; snprintf(b, sizeof(b), "%ld", v); SimPinState::serialBuffer.append(b); }
void SimSerial::print(unsigned long v) { char b[24]; snprintf(b, sizeof(b), "%lu", v); SimPinState::serialBuffer.append(b); }
void SimSerial::print(double v)        { char b[32]; snprintf(b, sizeof(b), "%g",  v); SimPinState::serialBuffer.append(b); }

void SimSerial::println()                          { SimPinState::serialBuffer.push_back('\n'); }
void SimSerial::println(const char* s)             { print(s); println(); }
void SimSerial::println(const __FlashStringHelper* s) { print(s); println(); }
void SimSerial::println(int v)                     { print(v); println(); }
void SimSerial::println(unsigned int v)            { print(v); println(); }
void SimSerial::println(long v)                    { print(v); println(); }
void SimSerial::println(unsigned long v)           { print(v); println(); }
void SimSerial::println(double v)                  { print(v); println(); }

void SimSerial::flush() {}
