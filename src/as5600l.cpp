#include "as5600l.h"

const uint8_t RAW_ANGLE = 0x0C;

AS5600L::AS5600L(I2C &i2c, uint8_t deviceAddress) : _i2c(i2c), _deviceAddress(deviceAddress) {}

bool AS5600L::Connected(void)
{
return _i2c.isConnected(_deviceAddress);
}
