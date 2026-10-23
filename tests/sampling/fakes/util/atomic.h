#pragma once
// Host tests are single threaded. Real AVR builds use avr-libc's atomic block.
#define ATOMIC_RESTORESTATE 0
#define ATOMIC_BLOCK(state) for (bool once = true; once; once = false)
