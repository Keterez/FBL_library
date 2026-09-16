#include "as5600l.h"

constexpr uint8_t RAW_ANGLE = 0x0C;
constexpr uint8_t STATUS = 0x0B;
constexpr uint8_t ANGLE = 0x0E;


constexpr uint8_t ZPOS_HIGH = 0x01;
constexpr uint8_t ZPOS_LOW = 0x02;

AS5600L::AS5600L(I2C &i2c, uint8_t deviceAddress) : _i2c(i2c) {}

bool AS5600L::begin(void)
{
    if (_i2c.isConnected(0x40))
    {
        _deviceAddress = 0x40;
        return true;
    }

    if (_i2c.isConnected(0x36))
    {
        _deviceAddress = 0x36;
        return true;
    }

    return false;
}

bool AS5600L::calibrate(void)
{
    uint16_t raw = readRaw();

    uint8_t highByte = (raw >> 8) & 0x0F;
    uint8_t lowByte = raw & 0xFF;

    if (!_i2c.writeRegister(_deviceAddress, ZPOS_HIGH, highByte))
    {
        return false;
    }

    if (!_i2c.writeRegister(_deviceAddress, ZPOS_LOW, lowByte))
    {
        return false;
    }

    delay(10);

    return true;
}

bool AS5600L::isConnected(void)
{
    return _i2c.isConnected(_deviceAddress);
}

MagnetStatus AS5600L::detectMagnet(void)
{

    uint8_t status;
    if (!_i2c.readRegister(_deviceAddress, STATUS, status, 1))
        return MagnetStatus::CommunicationError;

    if (status << 3)
        return MagnetStatus::TooStrong;

    if (status << 4)
        return MagnetStatus::TooWeak;

    if (status << 5)
        return MagnetStatus::Good;

    return MagnetStatus::NotDetected;
}

uint16_t AS5600L::readRaw(void)
{
    uint8_t raw[2];
    _i2c.readRegister(_deviceAddress, RAW_ANGLE, raw, 2);

    return ((uint16_t)(raw[0] & 0x0F) << 8) | raw[1];
}

uint16_t AS5600L::readAngle(void)
{
    uint8_t angle[2];
    _i2c.readRegister(_deviceAddress, ANGLE, angle, 2);

    return ((uint16_t)(raw[0] & 0x0F) << 8) | angle[1];
}

float AS5600L::readAngleDeg(void)
{
    return readAngle() * 360.0f / 4096.0f;
}

float AS5600L::readAngleRad(void)
{
    return readAngle() * (2.0f * PI_F) / 4096.0f;
}
