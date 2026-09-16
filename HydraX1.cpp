#include "HydraX1.h"
#include "platforms/Platform.h"

HydraX1Class::HydraX1Class() : _i2c(getPrimaryI2C()), _as(_i2c);

void HydraX1Class::begin
{
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

    pinMode(M1, OUTPUT);
}

bool HydraX1Class::calibrate(void)
{
    _as.calibrate();

    for (int i = 0; i < 3; i++)
    {                       // Simple sound indication of successful calibration 3 beeps
        Platform::analogWritePin(M1, 1); // Actuator powereded just a bit so the rotor doesn't turn just beep
        Platform::delayMs(200);         // wait
        Platform::analogWritePin(M1, 0); // Actuator powered off
        Platform::delayMs(200);         // wait
    }
}

float HydraX1Class::readSensor(void)
{
    _as.readAngleRad();
}

void HydraX1Class::actuatorWrite(float u)
{
    saturationFloat(u, _min_voltage, _max_voltage);
    Platform::analogWritePin(M1, input);
}

HydraX1Class HydraX1;