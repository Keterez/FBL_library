#ifndef arduinoI2C_H
#define arduinoI2C_H

#include <Wire.h>

class ArduinoI2C
{
public:
    ArduinoI2C(TwoWire &wire);
    bool begin(void);
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
}

#endif