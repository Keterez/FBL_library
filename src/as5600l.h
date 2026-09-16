#ifndef AS5600L_H
#define AS5600L_H

#include "I2C.h"
#include "constatns.h"
#include <stdint.h>

enum class MagnetStatus : uint8_t
{
    NotDetected,
    TooWeak,
    Good,
    TooStrong,
    CommunicationError
}

class AS5600L
{
public:
    AS5600L(I2C &i2c);

    bool begin(void);
    bool calibrate(void);
    bool isConnected(void);
    MagnetStatus detectMagnet(void);

    uint16_t readRaw(void);
    uint16_t readAngle(void); // read calibrated raw value from sensor
    float readAngleDeg(void);
    float readAngleRad(void);


private:
    I2C &_i2c;
    uint8_t _deviceAddress;
};

#endif