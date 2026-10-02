#ifndef AERO_H
#define AERO_H

#include "as5600l.h"
#include <stdint.h>

constexpr uint32_t AERO_MOTOR_PIN = 5;

class AeroClass
{
public:
    AeroClass();

    void begin();
    bool calibrate(void);
    float readSensor();
    void writeInput(float u);

private:
    I2C & _i2c;
    AS5600L _as;

    static constexpr float _min_voltage = 0.0f;
    static constexpr float _max_voltage = 3.0f;
};

extern AeroClass Aero;

#endif
