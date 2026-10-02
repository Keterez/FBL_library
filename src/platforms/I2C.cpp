#include "I2C.h"

#if defined(ARDUINO)

I2C& getPrimaryI2C(){
    static I2C bus(Wire);
    return bus;
}

I2C& getSecondaryI2C(){
#if defined(WIRE_INTERFACES_COUNT) && WIRE_INTERFACES_COUNT > 1
    static I2C bus(Wire1);
#else
    static I2C bus(Wire);
#endif
    return bus;
}

#endif
