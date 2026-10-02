#ifndef I2C_H
#define I2C_H
#if defined(ARDUINO) //Arduino symbol is set automaticly by Arduino IDE
// This section is used when compiling for Arduino.
#include "arduino/arduinoI2C.h"
using I2C = ArduinoI2C;

#else

#error "Unsupported platform: the STM32 I2C adapter is not implemented yet"

#endif

I2C& getPrimaryI2C();

I2C& getSecondaryI2C();

#endif
