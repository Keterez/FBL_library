#ifndef SYSEL_STM32_PLATFORM_H
#define SYSEL_STM32_PLATFORM_H

#include "main.h"
#include <cstring>

extern UART_HandleTypeDef huart2;

class STM32Platform
{
public:
    static void println(const char *text)
    {
        HAL_UART_Transmit(
            &huart2,
            reinterpret_cast<const uint8_t *>(text),
            std::strlen(text),
            HAL_MAX_DELAY);

        static const uint8_t newline[] = "\r\n";
        HAL_UART_Transmit(
            &huart2,
            newline,
            sizeof(newline) - 1,
            HAL_MAX_DELAY);
    }

    static void delayMs(uint32_t milliseconds)
    {
        HAL_Delay(milliseconds);
    }

    static void pinModeOutput(uint32_t)
    {
        // Piny sa pri CubeIDE zvyčajne konfigurujú cez CubeMX.
    }

    static void analogWritePin(uint32_t, uint32_t value)
    {
        // Tu nastavíš PWM cez HAL_TIM_PWM + compare register.
    }
};

#endif