#ifndef AS5600L_H
#define AS5600L_H

#include "I2C.h"
I2C i2c

class AS5600L
{
public:
    bool begin(void);
    bool Connected(void);
    bool detectMagnet(void);

    bool calibrate(void);
    
};

#endif