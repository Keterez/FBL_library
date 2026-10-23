#ifndef FBL_SAMPLING_H
#define FBL_SAMPLING_H

#include <Arduino.h>
#include <stdint.h>

enum class SamplingMode : uint8_t { Auto, Hardware, Software };
enum class SamplingError : uint8_t
{
    None,
    InvalidPeriod,
    InvalidCallback,
    HardwareUnavailable,
    TimerBusy,
    Overrun
};

// Call begin()/update()/stop() from loop context, with interrupts enabled.
// Auto uses a hardware timer on Uno/Mega 2560/Due, micros() elsewhere.
// The ISR only signals a release; both callbacks always run in update().
class SamplingClass
{
public:
    using Callback = void (*)();
    using Clock = uint32_t (*)();

    explicit SamplingClass(Clock clock = nullptr);
    ~SamplingClass();
    SamplingClass(const SamplingClass &) = delete;
    SamplingClass &operator=(const SamplingClass &) = delete;

    // Period in microseconds, 1..INT32_MAX. Hardware has additional limits.
    // First release is one full period after begin(). No catch-up bursts.
    // A failed configuration stops this instance; inspect error() for the reason.
    // Calls to begin() inside either callback are refused without changing state.
    bool begin(uint32_t periodUs, Callback step, Callback onOverrun = nullptr,
               SamplingMode mode = SamplingMode::Auto);
    bool update(); // true if a step was dispatched, even if it stopped sampling.
    void stop();  // Allowed inside the step; begin() must be outside callbacks.

    bool running() const { return _running; }
    SamplingError error() const { return _error; }
    SamplingMode mode() const { return _mode; }
    uint32_t periodUs() const { return _periodUs; }
    float periodSeconds() const { return _periodUs * 1.0e-6f; }
    uint32_t sampleCount() const { return _sampleCount; }
    uint32_t lastExecutionUs() const { return _lastExecutionUs; }
    uint32_t maxExecutionUs() const { return _maxExecutionUs; }
    uint32_t lastLatenessUs() const { return _lastLatenessUs; }

    // Internal timer hook. Not an application callback.
    void signalFromTimer();

private:
    bool startTimer();
    void stopTimer();
    uint8_t pendingReleases() const;
    uint8_t takeReleases();
    void overrun();

    Clock _clock;
    Callback _step = nullptr;
    Callback _onOverrun = nullptr;
    SamplingMode _mode = SamplingMode::Software;
    SamplingError _error = SamplingError::None;
    bool _running = false;
    bool _updating = false;
    volatile uint8_t _pending = 0; // Saturates at two: enough to detect a miss.
    uint32_t _periodUs = 0;
    uint32_t _nextUs = 0;
    uint32_t _sampleCount = 0;
    uint32_t _lastExecutionUs = 0;
    uint32_t _maxExecutionUs = 0;
    uint32_t _lastLatenessUs = 0;
};

extern SamplingClass Sampling;

#endif
