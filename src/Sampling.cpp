#include "FBLSampling.h"

// Backend selection is private; application code uses the same API everywhere.
// Define FBL_SAMPLING_SOFTWARE_ONLY for the entire build to omit timer ISRs.
#if !defined(FBL_SAMPLING_SOFTWARE_ONLY) && (defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_MEGA2560))
#define FBL_SAMPLING_AVR
#include <avr/interrupt.h>
#include <util/atomic.h>
#elif !defined(FBL_SAMPLING_SOFTWARE_ONLY) && defined(ARDUINO_ARCH_SAM)
#define FBL_SAMPLING_DUE
#endif

namespace
{
uint32_t arduinoMicros() { return static_cast<uint32_t>(micros()); }

#if defined(FBL_SAMPLING_AVR) || defined(FBL_SAMPLING_DUE)
SamplingClass *volatile timerOwner = nullptr;
constexpr bool hardwareAvailable = true;
#else
constexpr bool hardwareAvailable = false;
#endif

#if defined(FBL_SAMPLING_DUE)
class InterruptLock
{
public:
    InterruptLock() : saved(__get_PRIMASK()) { __disable_irq(); }
    ~InterruptLock() { __set_PRIMASK(saved); }
private:
    uint32_t saved;
};
#endif
}

SamplingClass Sampling;

SamplingClass::SamplingClass(Clock clock) : _clock(clock ? clock : arduinoMicros) {}
SamplingClass::~SamplingClass() { stop(); }

bool SamplingClass::begin(uint32_t periodUs, Callback step, Callback onOverrun, SamplingMode mode)
{
    // Refuse a restart while update() is still dispatching a callback.
    if (_updating) return false;
    stop();
    _error = SamplingError::None;
    _sampleCount = _lastExecutionUs = _maxExecutionUs = _lastLatenessUs = 0;
    _periodUs = periodUs;
    _step = step;
    _onOverrun = onOverrun;
    if (periodUs == 0 || periodUs > 0x7FFFFFFFUL)
    {
        _error = SamplingError::InvalidPeriod;
        return false;
    }
    if (!step)
    {
        _error = SamplingError::InvalidCallback;
        return false;
    }
    _mode = mode == SamplingMode::Auto
        ? (hardwareAvailable ? SamplingMode::Hardware : SamplingMode::Software) : mode;
    _pending = 0;
    if (_mode == SamplingMode::Hardware)
    {
        if (!startTimer()) return false;
    }
    else
    {
        _nextUs = _clock() + _periodUs;
    }
    _running = true;
    return true;
}

void SamplingClass::stop()
{
    if (_mode == SamplingMode::Hardware) stopTimer();
    _running = false;
    _pending = 0;
}

void SamplingClass::overrun()
{
    stop();
    _error = SamplingError::Overrun;
    if (_onOverrun) _onOverrun();
}

bool SamplingClass::update()
{
    if (!_running || _updating) return false;
    const uint8_t releases = _mode == SamplingMode::Hardware ? takeReleases() : 0;
    if (_mode == SamplingMode::Hardware && releases == 0) return false;
    const uint32_t now = _clock();
    const int32_t lateness = static_cast<int32_t>(now - _nextUs);
    if (_mode == SamplingMode::Software && lateness < 0) return false;
    _updating = true;
    if (lateness >= static_cast<int32_t>(_periodUs) ||
        (_mode == SamplingMode::Hardware && releases > 1))
    {
        overrun();
        _updating = false;
        return false;
    }
    _lastLatenessUs = lateness > 0 ? static_cast<uint32_t>(lateness) : 0;
    _nextUs += _periodUs; // Preserve phase rather than scheduling from "now".
    _step();
    const uint32_t end = _clock();
    _lastExecutionUs = end - now;
    if (_lastExecutionUs > _maxExecutionUs) _maxExecutionUs = _lastExecutionUs;
    ++_sampleCount;
    if (_running && (static_cast<int32_t>(end - _nextUs) >= 0 ||
                     (_mode == SamplingMode::Hardware && pendingReleases() != 0)))
    {
        overrun();
    }
    _updating = false;
    return true;
}

void SamplingClass::signalFromTimer()
{
    if (_pending < 2) ++_pending;
}

uint8_t SamplingClass::pendingReleases() const
{
    // Single-byte reads are atomic on all supported targets.
    return _pending;
}

uint8_t SamplingClass::takeReleases()
{
    uint8_t releases = 0;
    // A release arriving after this snapshot remains visible during the step.
#if defined(FBL_SAMPLING_AVR)
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE) { releases = _pending; _pending = 0; }
#elif defined(FBL_SAMPLING_DUE)
    InterruptLock lock;
    releases = _pending;
    _pending = 0;
#endif
    return releases;
}

bool SamplingClass::startTimer()
{
#if defined(FBL_SAMPLING_AVR) || defined(FBL_SAMPLING_DUE)
    if (timerOwner && timerOwner != this)
    {
        _error = SamplingError::TimerBusy;
        return false;
    }
#endif
#if defined(FBL_SAMPLING_AVR)
    const uint16_t divisors[] = {1, 8, 64, 256, 1024};
    const uint64_t cycles = static_cast<uint64_t>(F_CPU) * _periodUs;
    uint32_t ticks = 0;
    uint8_t clockBits = 0;
    for (uint8_t i = 0; i < 5; ++i)
    {
        const uint32_t denominator = static_cast<uint32_t>(divisors[i]) * 1000000UL;
        const uint64_t count = cycles / denominator;
        if (cycles % denominator == 0 && count >= 1 && count <= 65536UL)
        {
            ticks = static_cast<uint32_t>(count);
            clockBits = i + 1;
            break;
        }
    }
    if (!ticks)
    {
        _error = SamplingError::InvalidPeriod;
        return false;
    }
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        timerOwner = this;
#if defined(ARDUINO_AVR_UNO)
        TCCR1A = 0;
        TCCR1B = 0;
        TIMSK1 = 0;
        TCNT1 = 0;
        OCR1A = static_cast<uint16_t>(ticks - 1);
        TIFR1 = _BV(OCF1A);
        TIMSK1 = _BV(OCIE1A);
        _nextUs = _clock() + _periodUs;
        TCCR1B = _BV(WGM12) | clockBits;
#else
        TCCR5A = 0;
        TCCR5B = 0;
        TIMSK5 = 0;
        TCNT5 = 0;
        OCR5A = static_cast<uint16_t>(ticks - 1);
        TIFR5 = _BV(OCF5A);
        TIMSK5 = _BV(OCIE5A);
        _nextUs = _clock() + _periodUs;
        TCCR5B = _BV(WGM52) | clockBits;
#endif
    }
    return true;
#elif defined(FBL_SAMPLING_DUE)
    const uint64_t numerator = static_cast<uint64_t>(VARIANT_MCK) * _periodUs;
    const uint64_t ticks = numerator / 2000000UL;
    if (numerator % 2000000UL || ticks < 1 || ticks > 0xFFFFFFFFULL)
    {
        _error = SamplingError::InvalidPeriod;
        return false;
    }
    InterruptLock lock;
    timerOwner = this;
    pmc_set_writeprotect(false);
    pmc_enable_periph_clk(ID_TC5);
    NVIC_DisableIRQ(TC5_IRQn);
    TC_Configure(TC1, 2, TC_CMR_WAVE | TC_CMR_WAVSEL_UP_RC | TC_CMR_TCCLKS_TIMER_CLOCK1);
    TC_SetRC(TC1, 2, static_cast<uint32_t>(ticks));
    TC_GetStatus(TC1, 2);
    NVIC_ClearPendingIRQ(TC5_IRQn);
    TC1->TC_CHANNEL[2].TC_IER = TC_IER_CPCS;
    NVIC_EnableIRQ(TC5_IRQn);
    _nextUs = _clock() + _periodUs;
    TC_Start(TC1, 2);
    return true;
#else
    _error = SamplingError::HardwareUnavailable;
    return false;
#endif
}

void SamplingClass::stopTimer()
{
#if defined(FBL_SAMPLING_AVR)
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if (timerOwner == this)
        {
#if defined(ARDUINO_AVR_UNO)
            TIMSK1 = 0;
            TCCR1B = 0;
            TIFR1 = _BV(OCF1A);
#else
            TIMSK5 = 0;
            TCCR5B = 0;
            TIFR5 = _BV(OCF5A);
#endif
            timerOwner = nullptr;
        }
    }
#elif defined(FBL_SAMPLING_DUE)
    InterruptLock lock;
    if (timerOwner == this)
    {
        NVIC_DisableIRQ(TC5_IRQn);
        TC1->TC_CHANNEL[2].TC_IDR = 0xFFFFFFFFUL;
        TC_Stop(TC1, 2);
        TC_GetStatus(TC1, 2);
        NVIC_ClearPendingIRQ(TC5_IRQn);
        timerOwner = nullptr;
    }
#endif
}

#if defined(FBL_SAMPLING_AVR)
#if defined(ARDUINO_AVR_UNO)
ISR(TIMER1_COMPA_vect)
#else
ISR(TIMER5_COMPA_vect)
#endif
{
    if (timerOwner) timerOwner->signalFromTimer();
}
#elif defined(FBL_SAMPLING_DUE)
void TC5_Handler()
{
    TC_GetStatus(TC1, 2);
    if (timerOwner) timerOwner->signalFromTimer();
}
#endif
