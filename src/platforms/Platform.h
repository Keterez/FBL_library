#ifndef SYSEL_PLATFORM_H
#define SYSEL_PLATFORM_H

#if defined(ARDUINO)

#include "arduino/ArduinoPlatform.h"
using Platform = ArduinoPlatform;

#elif defined(SYSEL_STM32)

#include "stm32/STM32Platform.h"
using Platform = STM32Platform;

#else
#error "Unsupported platform"
#endif

#endif