#include "arduinoI2C.h"

ArduinoI2C::ArduinoI2C(TwoWire &wire) : _wire(wire) {}

void ArduinoI2C::begin(void)
{
    _wire.begin(); // Begin wire communiciation
}

bool ArduinoI2C::isConnected(uint8_t deviceAddress){
    _wire.beginTransmission(deviceAddress);

    return _wire.endTrasmission() == 0;
}

bool ArduinoI2C::readRegister(
    uint8_t deviceAddress,
    uint8_t registerAddress,
    uint8_t *data,
    uint8_t length)
{
    _wire.beginTransmission(deviceAddress);
    // Start communication with the I2C device at the specified address.

    _wire.write(registerAddress);
    // Send the register address from which we want to start reading.

    if (_wire.endTransmission(false) != 0)
    {
        // Send the register address without generating a STOP condition,
        // because a read operation follows immediately using a repeated START.
        return false;
    }

    uint8_t received = _wire.requestFrom(deviceAddress, length);
    // Request 'length' bytes from the selected I2C device.
    // The received bytes are stored in the internal Wire RX buffer.

    if (received != length)
    {
        return false;
    }

    for (uint8_t i = 0; i < length; i++)
    {
        data[i] = _wire.read();
        // Read one byte from the Wire RX buffer.
        // Each subsequent call returns the next byte.
    }

    return true;
}

bool ArduinoI2C::writeRegister(
    uint8_t deviceAddress,
    uint8_t registerAddress,
    uint8_t value)
{
    _wire.beginTransmission(deviceAddress);
    // Start communication with the I2C device at the specified address.

    _wire.write(registerAddress);
    // Send the address of the register where the value should be written.

    _wire.write(value);
    // Add the value that should be written into the selected register.

    return _wire.endTransmission() == 0;
    // Send the complete transmission.
    // Return true if the device acknowledged the transmission, otherwise false.
}