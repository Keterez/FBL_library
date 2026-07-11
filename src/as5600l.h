#ifndef AS5600L_H
#define AS5600L_H

#include "I2C.h"
#include <stdint.h>


class AS5600L
{
public:
    AS5600L(I2C &i2c, uint8_t deviceAddress);
    bool isConnected(void);
    bool detectMagnet(void);

    bool calibrate(void);

private:
    I2C &_i2c;
    uint8_t _deviceAddress;
};

#endif