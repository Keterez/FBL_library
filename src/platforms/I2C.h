#ifndef I2C_H

/*
#if defined(Arduino) //Arduino symbol is set automaticly by Arduino IDE
// this will called olny if Arduino IDE is used 
#include "arduinoI2C.h"
using I2C = ArduinoI2C;

#elif defined(SYSEL_STM32) //SYSEL_STM32 must be define by user manualy, when STM32CubeIde is used
//This section is called when STM32CubeIDE is used
#include "stm32I2C.h"
using I2C = STM32I2C;
#endif
*/

#include "arduinoI2C.h"
using I2C = ArduinoI2C;

#endif