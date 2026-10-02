#include "as5600l.h"
#include "platforms/Platform.h"

constexpr uint8_t RAW_ANGLE = 0x0C;
constexpr uint8_t STATUS = 0x0B;
constexpr uint8_t ANGLE = 0x0E;


constexpr uint8_t ZPOS_HIGH = 0x01;
constexpr uint8_t ZPOS_LOW = 0x02;

AS5600L::AS5600L(I2C &i2c) : _i2c(i2c), _deviceAddress(0) {}

bool AS5600L::begin(void)
{
    _i2c.begin();

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
    uint16_t raw;
    if (!read12BitRegister(RAW_ANGLE, raw))
    {
        return false;
    }

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

    Platform::delayMs(10);

    return true;
}

bool AS5600L::isConnected(void)
{
    return _deviceAddress != 0 && _i2c.isConnected(_deviceAddress);
}

MagnetStatus AS5600L::detectMagnet(void)
{

    uint8_t status = 0;
    if (!_i2c.readRegister(_deviceAddress, STATUS, &status, 1))
        return MagnetStatus::CommunicationError;

    if ((status & (1U << 3)) != 0)
        return MagnetStatus::TooStrong;

    if ((status & (1U << 4)) != 0)
        return MagnetStatus::TooWeak;

    if ((status & (1U << 5)) != 0)
        return MagnetStatus::Good;

    return MagnetStatus::NotDetected;
}

uint16_t AS5600L::readRaw(void)
{
    uint16_t value = 0;
    read12BitRegister(RAW_ANGLE, value);
    return value;
}

uint16_t AS5600L::readAngle(void)
{
    uint16_t value = 0;
    read12BitRegister(ANGLE, value);
    return value;
}

bool AS5600L::read12BitRegister(uint8_t registerAddress, uint16_t &value)
{
    uint8_t data[2] = {0, 0};
    if (!_i2c.readRegister(_deviceAddress, registerAddress, data, 2))
    {
        value = 0;
        return false;
    }

    value = (static_cast<uint16_t>(data[0] & 0x0F) << 8) | data[1];
    return true;
}

float AS5600L::readAngleDeg(void)
{
    return readAngle() * 360.0f / 4096.0f;
}

float AS5600L::readAngleRad(void)
{
    return readAngle() * (2.0f * PI_F) / 4096.0f;
}
