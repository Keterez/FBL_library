#pragma once
#include <stdint.h>
uint32_t micros();

#if defined(ARDUINO_AVR_UNO)
// Register stand-ins for deterministic host tests of the AVR backend.
extern uint8_t TCCR1A, TCCR1B, TIMSK1, TIFR1;
extern uint16_t TCNT1, OCR1A;
#define _BV(bit) (1U << (bit))
#define OCF1A 1
#define OCIE1A 1
#define WGM12 3
#endif
