#ifndef ARDUINO_I2C_H
#define ARDUINO_I2C_H

#include <Wire.h>
#include <stdint.h>

class ArduinoI2C
{
public:
    ArduinoI2C(TwoWire &wire);
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
    TwoWire &_wire;
};

#endif