src/
│
├── sensors/
│   ├── AS5600L.h
│   └── AS5600L.cpp
│
├── communication/
│   └── I2C.h
│
└── platforms/
    ├── arduino/
    │   ├── ArduinoI2C.h
    │   └── ArduinoI2C.cpp
    │
    └── stm32/
        ├── STM32I2C.h
        └── STM32I2C.cpp

                  AS5600L #1
                       │
                  AS5600L #2
                       │
                      IMU
                       │
                       ▼
                 spoločné I2C API
                       │
             ┌─────────┴─────────┐
             ▼                   ▼
       ArduinoI2C            STM32I2C
             │                   │
           Wire                hi2c1

        #include "AS5600L.h"
#include <Wire.h>

#define AS5600L_RAW_ANGLE_H  0x0C
#define AS5600L_RAW_ANGLE_L  0x0D

AS5600L::AS5600L(uint8_t address)
{
    _address = address;
}

uint16_t AS5600L::readRaw()
{
    Wire.beginTransmission(_address);
    Wire.write(AS5600L_RAW_ANGLE_H);
    Wire.endTransmission(false);

    Wire.requestFrom(_address, (uint8_t)2);

    if (Wire.available() < 2)
    {
        return 0;
    }

    uint8_t highByte = Wire.read();
    uint8_t lowByte  = Wire.read();

    uint16_t rawValue =
        ((uint16_t)(highByte & 0x0F) << 8) |
        lowByte;

    return rawValue;
}


uint16_t raw = ((uint16_t)(data[0] & 0x0F) << 8) | data[1];

bool ArduinoI2C::readRegisters(
    uint8_t deviceAddress,
    uint8_t registerAddress,
    uint8_t* data,
    uint8_t length)
{
    _wire.beginTransmission(deviceAddress);
    _wire.write(registerAddress);

    if (_wire.endTransmission(false) != 0)
    {
        return false;
    }

    uint8_t received = _wire.requestFrom(deviceAddress, length);

    if (received != length)
    {
        return false;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        data[i] = _wire.read();
    }

    return true;
}