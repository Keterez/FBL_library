#ifndef AERO_H
#define AERO_H

#include "as5600l.h"
#include <stdint.h>
#include <Arduino.h>

#define AeroRelease 1

constexpr uint32_t AERO_MOTOR_PIN = 5;
constexpr uint32_t START = 10;
constexpr uint32_t USER = 9;

constexpr uint32_t BIT1 = 8;
constexpr uint32_t BIT2 = 7;
constexpr uint32_t BIT3 = 6;

constexpr uint32_t POT1 = A3;

constexpr uint32_t GREEN_LED = 13;
constexpr uint32_t RED_LED = 12;
constexpr uint32_t BLUE_LED = 11;

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
    void turnOnGreen();
    void turnOnRed();
    void turnOnBlue();
    void turnOffLed();


    static constexpr float _min_voltage = 0.0f;
    static constexpr float _max_voltage = 3.0f;
};

extern AeroClass Aero;

#endif
