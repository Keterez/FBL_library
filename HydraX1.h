#ifndef HYDRAX1_H
#define HYDRAX1_H

#include "as5600l.h"

#define M1 5

class HydraX1Class
{
public:
    void begin();
    bool calibrate(void);
    float readSensor();
    void writeInput(float u);

private:
    I2C & _i2c;
    AS5600L _as;

    _min_voltage = 0.0;
    _max_voltage = 3.0;
}

#endif