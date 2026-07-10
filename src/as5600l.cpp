#include "as5600l.h"

const uint8_t RAW_ANGLE = 0x0C;

bool AS5600L::begin(void)
{
    i2c.begin();
}

