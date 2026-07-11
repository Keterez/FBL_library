#include "I2C.h"

#if defined(ARDUINO)

I2C& getPrimaryI2C(){
    static I2C bus(Wire);
    return bus;
}

I2C& getSecondaryI2C(){
    static I2C bus(Wire1);
    return bus;
}

#elif defined(SYSEL_STM32)

extern I2C_HandleTypeDef hi2c1;
extern I2C_HandleTypeDef hi2c2;


I2C& getPrimaryI2C(){
    static I2C bus(hi2c1);
    return bus;
}

I2C& getSecondaryI2C(){
    static I2C bus(hi2c2);
    return bus;
}


#endif