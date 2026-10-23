#include "FBLSampling.h"
#include <assert.h>
#include <stdio.h>

uint32_t nowUs = 0;
uint32_t micros() { return nowUs; }
uint32_t workUs = 0;
unsigned calls = 0, faults = 0;
SamplingClass *active = nullptr;
bool stopInStep = false, recurseInStep = false;

void step()
{
    ++calls;
    if (recurseInStep) assert(!active->update());
    nowUs += workUs;
    if (stopInStep) active->stop();
}

void fault()
{
    ++faults;
    assert(!active->running());
    assert(active->error() == SamplingError::Overrun);
    assert(!active->update());
}

void reset(SamplingClass &sampler)
{
    sampler.stop();
    active = &sampler;
    nowUs = workUs = calls = faults = 0;
    stopInStep = recurseInStep = false;
}

#if defined(ARDUINO_AVR_UNO)
uint8_t TCCR1A = 0, TCCR1B = 0, TIMSK1 = 0, TIFR1 = 0;
uint16_t TCNT1 = 0, OCR1A = 0;
void TIMER1_COMPA_vect();

void hardwareTests()
{
    SamplingClass sampler, other;
    reset(sampler);
    assert(sampler.begin(5000, step, fault));
    assert(sampler.mode() == SamplingMode::Hardware);
    assert(OCR1A == 9999); // 16 MHz / 8 * 5 ms, then subtract one.
    assert(TCCR1B == (_BV(WGM12) | 2));
    assert(!other.begin(5000, step));
    assert(other.error() == SamplingError::TimerBusy);
    other.stop(); // Must not stop another instance's timer.
    assert(TIMSK1 == _BV(OCIE1A));
    nowUs = 5000;
    assert(!sampler.update()); // Hardware release required even if micros is due.
    TIMER1_COMPA_vect();
    assert(sampler.update());
    assert(calls == 1);
    nowUs = 15000;
    TIMER1_COMPA_vect();
    TIMER1_COMPA_vect();
    TIMER1_COMPA_vect(); // Saturating counter cannot wrap back to zero.
    assert(!sampler.update());
    assert(calls == 1 && faults == 1 && TIMSK1 == 0 && TCCR1B == 0);
    assert(other.begin(10000, step)); // Ownership released after failure.
    other.stop();

    reset(sampler);
    assert(!sampler.begin(4194305UL, step));
    assert(sampler.error() == SamplingError::InvalidPeriod);
    assert(sampler.begin(4000000UL, step));
    assert(OCR1A == 62499 && TCCR1B == (_BV(WGM12) | 5));
    sampler.stop();

    reset(sampler);
    assert(sampler.begin(5000, step, fault));
    nowUs = 5000;
    TIMER1_COMPA_vect();
    workUs = 5000;
    assert(sampler.update());
    assert(faults == 1 && !sampler.running());
}
#endif

int main()
{
    SamplingClass sampler;
    reset(sampler);
    assert(!sampler.begin(0, step));
    assert(sampler.error() == SamplingError::InvalidPeriod);
    assert(!sampler.begin(0x80000000UL, step));
    assert(!sampler.begin(5000, nullptr));
    assert(sampler.error() == SamplingError::InvalidCallback);
    assert(!sampler.running());

    // No early calls, no duplicate calls; mild lateness must not shift phase.
    assert(sampler.begin(5000, step, fault, SamplingMode::Software));
    assert(sampler.periodUs() == 5000);
    assert(sampler.periodSeconds() > 0.004999f && sampler.periodSeconds() < 0.005001f);
    assert(!sampler.update());
    nowUs = 4999;
    assert(!sampler.update());
    nowUs = 5100;
    workUs = 200;
    assert(sampler.update() && calls == 1);
    assert(sampler.lastLatenessUs() == 100 && sampler.lastExecutionUs() == 200);
    assert(!sampler.update());
    nowUs = 9999;
    assert(!sampler.update());
    nowUs = 10000;
    workUs = 50;
    assert(sampler.update() && calls == 2);
    assert(sampler.lastLatenessUs() == 0 && sampler.maxExecutionUs() == 200);
    assert(sampler.sampleCount() == 2);

    // Missed release: stop before executing a stale sample; notify once.
    nowUs = 20000;
    assert(!sampler.update());
    assert(calls == 2 && faults == 1 && !sampler.running());
    assert(!sampler.update() && faults == 1);

    // Restart resets metrics/errors and schedules a full first period.
    nowUs = 25000;
    assert(sampler.begin(5000, step, fault, SamplingMode::Software));
    assert(sampler.error() == SamplingError::None && sampler.sampleCount() == 0);
    assert(sampler.maxExecutionUs() == 0);
    nowUs = 29999;
    assert(!sampler.update());
    nowUs = 30000;
    workUs = 5000;
    assert(sampler.update());
    assert(!sampler.running() && faults == 2); // Equality is a missed deadline.

    // Time spent late plus work, rather than work duration alone, sets deadline.
    reset(sampler);
    assert(sampler.begin(5000, step, fault, SamplingMode::Software));
    nowUs = 9000;
    workUs = 1000;
    assert(sampler.update() && faults == 1);

    // micros() rollover, including the execution-duration subtraction.
    reset(sampler);
    nowUs = 0xFFFFFFF0UL;
    assert(sampler.begin(20, step, fault, SamplingMode::Software));
    nowUs = 3;
    assert(!sampler.update());
    nowUs = 4;
    workUs = 2;
    assert(sampler.update() && calls == 1 && sampler.lastExecutionUs() == 2);
    nowUs = 24;
    assert(sampler.update() && calls == 2 && faults == 0);
    reset(sampler);
    nowUs = 0xFFFFFFF0UL;
    assert(sampler.begin(8, step, fault, SamplingMode::Software));
    nowUs = 0xFFFFFFFEUL;
    workUs = 3;
    assert(sampler.update() && sampler.lastExecutionUs() == 3 && faults == 1);

    // Callback can stop normally; recursive update() must never dispatch twice.
    reset(sampler);
    assert(sampler.begin(5000, step, fault, SamplingMode::Software));
    recurseInStep = stopInStep = true;
    nowUs = 5000;
    workUs = 10000;
    assert(sampler.update() && calls == 1 && faults == 0 && !sampler.running());

#if defined(ARDUINO_AVR_UNO)
    hardwareTests();
    puts("Sampling tests passed (software + simulated AVR timer).");
#else
    reset(sampler);
    assert(!sampler.begin(5000, step, fault, SamplingMode::Hardware));
    assert(sampler.error() == SamplingError::HardwareUnavailable);
    assert(sampler.begin(5000, step));
    assert(sampler.mode() == SamplingMode::Software);
    puts("Sampling tests passed (software + Auto fallback).");
#endif
}
