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

    Platform::pinModeOutput(GREEN_LED);
    Platform::pinModeOutput(RED_LED);
    Platform::pinModeOutput(BLUE_LED);

    Platform::pinModeOutput(AERO_MOTOR_PIN);
    Platform::pinModeInput(POT1);

    bool bit1 = Platform::digitalReadPin(BIT1);
    bool bit2 = Platform::digitalReadPin(BIT2);
    bool bit3 = Platform::digitalReadPin(BIT3);

    if (bit1 && !bit2 && !bit3)
    {
        if (AeroRelease == 1)
        {
            Platform::println("Aero is release R1.");
            turnOnGreen();
        }
    }
    else
    {
        turnOnRed();

        while (true)
        {
            Platform::println("Aero is not release R1! Please set correct release version in Aero.h.");
            Platform::delayMs(500);
        }
    }

    // Check whether the AS5600 sensor is connected.
    while (!_as.begin())
    {
        Platform::println("Potentiometer is not connected.");
        turnOnRed();
        Platform::delayMs(125);
        turnOffLed();
        Platform::delayMs(125);
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
        turnOnRed();
        Platform::delayMs(250);
        turnOffLed();
        Platform::delayMs(250);
    }
    turnOnBlue();
    Platform::println("Aero device initialized successfully.");
    Platform::delayMs(500);
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
    {
        // Simple sound indication of successful calibration 3 beeps, Led is also switching
        Platform::analogWritePin(AERO_MOTOR_PIN, 1); // Actuator powered just enough to beep without turning the rotor
        if (i == 0)
            turnOnGreen();
        else if (i == 1)
            turnOnGreen();
        else if (i == 2)
            turnOnRed();
        Platform::delayMs(200);                      // wait
        
        Platform::analogWritePin(AERO_MOTOR_PIN, 0); // Actuator powered off
        turnOffLed();
        Platform::delayMs(200); // wait
    }

    Platform::println("Calibration successful.");
    Platform::delayMs(500); // wait for the last beep to finish

    Platform::println("Press the START button to continue.");
    Platform::delayMs(250);

    while (Platform::digitalReadPin(START))
    {
        turnOnGreen();
        Platform::delayMs(125);
        turnOffLed();
        Platform::delayMs(125);

    }

    turnOnGreen();
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

void AeroClass::turnOnGreen()
{
    Platform::digitalWrite(GREEN_LED, LOW);
    Platform::digitalWrite(RED_LED, HIGH);
    Platform::digitalWrite(BLUE_LED, HIGH);
}

void AeroClass::turnOnRed()
{
    Platform::digitalWrite(GREEN_LED, HIGH);
    Platform::digitalWrite(RED_LED, LOW);
    Platform::digitalWrite(BLUE_LED, HIGH);
}

void AeroClass::turnOnBlue()
{
    Platform::digitalWrite(GREEN_LED, HIGH);
    Platform::digitalWrite(RED_LED, HIGH);
    Platform::digitalWrite(BLUE_LED, LOW);
}
void AeroClass::turnOffLed()
{
    Platform::digitalWrite(GREEN_LED, HIGH);
    Platform::digitalWrite(RED_LED, HIGH);
    Platform::digitalWrite(BLUE_LED, HIGH);
}
AeroClass Aero;
