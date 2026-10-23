/*
  Adapted for the Feedback Lab (FBL) Aero R1 API from
  AutomationShield/examples/AeroShield/AeroShield_Identification.
  Original authors: Jan Boldocky.
  Original license: Creative Commons Attribution-NonCommercial 4.0.
  https://creativecommons.org/licenses/by-nc/4.0/
  https://www.automationshield.com

  FBL: angle in radians, motor command 0..3 V (linear PWM mapping).
  START begins the experiment after calibration; USER stops the motor.
  Blue = ready/finished, green = running, red = fault.
  Serial Plotter: 115200 baud. No AutomationShield dependency.
*/

#include "aprbsU.h"  // Replace with aprbsU_val.h for validation.

#include <Aero.h>
#include <FBLSampling.h>
#include <math.h>

constexpr uint32_t SAMPLE_US = 10000;
constexpr float PI_RAD = 3.14159265358979323846f;
uint32_t sampleIndex = 0;
bool stopped = false;

void stopExperiment(bool error, const __FlashStringHelper *message) {
  Sampling.stop();
  Aero.writeInput(0.0f);
  Aero.setStatus(error ? AeroStatus::Error : AeroStatus::Ready);
  stopped = true;
  Serial.println(message);
}

// The sensor returns [0, 2*pi); unwrap around the calibrated rest position.
float readAngleRad() {
  float angle = Aero.readSensor();
  return angle > PI_RAD ? angle - 2.0f * PI_RAD : angle;
}

void step();
void samplingError() {
  stopExperiment(true, F("Sampling deadline missed."));
}

void setup() {
  Serial.begin(115200);
  Aero.begin();
  // Keep the arm at rest. Calibration waits for the START button.
  if (!Aero.calibrate()) {
    stopExperiment(true, F("Calibration failed."));
    return;
  }
  Serial.println(F("angle_rad, input_V"));
  Aero.setStatus(AeroStatus::Running);
  if (!Sampling.begin(SAMPLE_US, step, samplingError)) {
    stopExperiment(true, F("Cannot start sampling."));
  }
}

void loop() {
  if (stopped) return;
  if (digitalRead(USER) == LOW) {
    stopExperiment(false, F("Stopped by USER. Reset to restart."));
    return;
  }
  if (Sampling.update()) ++sampleIndex;
}

void step() {
  if (sampleIndex >= aprbsU_length) {
    stopExperiment(false, F("Identification finished."));
    return;
  }
  const float angle = readAngleRad();
  if (!isfinite(angle) || fabsf(angle) > 170.0f * PI_RAD / 180.0f) {
    stopExperiment(true, F("Angle limit exceeded."));
    return;
  }
#if defined(ARDUINO_ARCH_AVR)
  const float requested = pgm_read_float(&aprbsU[sampleIndex]);
#else
  const float requested = aprbsU[sampleIndex];
#endif
  const float voltage = constrain(requested, 0.0f, 3.0f);
  Aero.writeInput(voltage);
  Serial.print(angle, 5);
  Serial.print(',');
  Serial.println(voltage, 5);
}
