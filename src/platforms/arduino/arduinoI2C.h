#ifndef ARDUINO_I2C_H
#define ARDUINO_I2C_H

#include <Wire.h>
#include <stdint.h>

// Wire is namespaced on some Arduino cores (for example SAMD).
using ArduinoWire = decltype(Wire);

class ArduinoI2C
{
public:
    ArduinoI2C(ArduinoWire &wire);
    void begin(void);
    bool isConnected(uint8_t deviceAddress);
    bool readRegister(uint8_t deviceAddress,
                      uint8_t registerAddress,
                      uint8_t *data,
                      uint8_t length);
    bool writeRegister(
        uint8_t deviceAddress,
        uint8_t registerAddress,
        uint8_t value);

private:
    ArduinoWire &_wire;
};

#endif
