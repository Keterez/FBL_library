#include "Aero.h"
#include "platforms/Platform.h"

AeroClass::AeroClass() : _i2c(getPrimaryI2C()), _as(_i2c)
{
}

void AeroClass::begin()
{
    Platform::pinModeInputPullup(START);
    Platform::pinModeInputPullup(USER);
    Platform::pinModeInputPullup(BIT1);
    Platform::pinModeInputPullup(BIT2);
    Platform::pinModeInputPullup(BIT3);
    
    bool bit1 = Platform::digitalReadPin(BIT1);
    bool bit2 = Platform::digitalReadPin(BIT2);
    bool bit3 = Platform::digitalReadPin(BIT3);

    if (bit1 && bit2 && bit3)
    {
        Platform::println("Aero is release R1.");
    }
    
    // Check whether the AS5600 sensor is connected.
    while (!_as.begin())
    {
        Platform::println("Potentiometer is not connected.");
        Platform::delayMs(500);
    }

    // Check whether the magnet is correctly detected.
    while (_as.detectMagnet() != MagnetStatus::Good)
    {
        MagnetStatus status = _as.detectMagnet();

        switch (status)
        {
        case MagnetStatus::NotDetected:
            Platform::println("Magnet is not detected.");
            break;

        case MagnetStatus::TooWeak:
            Platform::println("Magnet is too far from the sensor.");
            break;

        case MagnetStatus::TooStrong:
            Platform::println("Magnet is too close to the sensor.");
            break;

        case MagnetStatus::CommunicationError:
            Platform::println("Communication error while checking magnet.");
            break;

        case MagnetStatus::Good:
            break;
        }

        Platform::delayMs(500);
    }
    Platform::println("AS5600 initialized successfully.");

    Platform::pinModeOutput(AERO_MOTOR_PIN);
}

bool AeroClass::calibrate(void)
{
    bool success = _as.calibrate();

    if (!success)
    {
        Platform::println("Calibration failed.");
        return false;
    }

    for (int i = 0; i < 3; i++)
    {                                    // Simple sound indication of successful calibration 3 beeps
        Platform::analogWritePin(AERO_MOTOR_PIN, 1); // Actuator powered just enough to beep without turning the rotor
        Platform::delayMs(200);          // wait
        Platform::analogWritePin(AERO_MOTOR_PIN, 0); // Actuator powered off
        Platform::delayMs(200);          // wait
    }

    Platform::println("Calibration successful.");
    Platform::delayMs(500); // wait for the last beep to finish

    Platform::println("Press the START button to continue.");
    while (true){
        if (Platform::digitalReadPin(START))
        {
            break;
        }
    }

    return true;
}

float AeroClass::readSensor(void)
{
    return _as.readAngleRad();
}

void AeroClass::writeInput(float u)
{
    if (u < _min_voltage)
    {
        u = _min_voltage;
    }
    else if (u > _max_voltage)
    {
        u = _max_voltage;
    }

    constexpr float pwmMaximum = 255.0f;
    const uint32_t pwmValue = static_cast<uint32_t>(
        ((u - _min_voltage) / (_max_voltage - _min_voltage)) * pwmMaximum + 0.5f);

    Platform::analogWritePin(AERO_MOTOR_PIN, pwmValue);
}

AeroClass Aero;
