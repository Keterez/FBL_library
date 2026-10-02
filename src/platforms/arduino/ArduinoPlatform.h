#ifndef SYSEL_ARDUINO_PLATFORM_H
#define SYSEL_ARDUINO_PLATFORM_H

#include <Arduino.h>

class ArduinoPlatform
{
public:
    static void println(const char *text)
    {
        Serial.println(text);
    }

    static void delayMs(uint32_t milliseconds)
    {
        delay(milliseconds);
    }

    static void pinModeOutput(uint32_t pin)
    {
        pinMode(pin, OUTPUT);
    }

    static void analogWritePin(uint32_t pin, uint32_t value)
    {
        analogWrite(pin, value);
    }

    static void pinMode(uint32_t pin, uint32_t mode)
    {
        ::pinMode(pin, mode);
    }

    static void digitalWrite(uint32_t pin, uint32_t value)
    {
        ::digitalWrite(pin, value);
    }

    static uint32_t analogReadPin(uint32_t pin)
    {
        return ::analogRead(pin);
    }
};

#endif
