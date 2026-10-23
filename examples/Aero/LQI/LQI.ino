/*
  Adapted for the Feedback Lab (FBL) Aero R1 API from
  AutomationShield/examples/AeroShield/AeroShield_LQI.
  Original authors: Jan Boldocky.
  Original license: Creative Commons Attribution-NonCommercial 4.0.
  https://creativecommons.org/licenses/by-nc/4.0/
  https://www.automationshield.com

  FBL: angle in radians, motor command 0..3 V (linear PWM mapping).
  START begins the experiment after calibration; USER stops the motor.
  Blue = ready/finished, green = running, red = fault.
  Serial Plotter: 115200 baud. No AutomationShield dependency.
*/

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
  Serial.println(F("reference_rad, angle_rad, input_V"));
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

#define MANUAL 0
const float references[] = {0.8f, 1.0f, 0.35f, 1.3f, 1.0f, 0.35f, 0.8f, 1.2f, 0.0f};
constexpr uint32_t SECTION_SAMPLES = 600;
float integralState = 0.0f;
float previousAngle = 0.0f;

void step() {
  float reference;
#if MANUAL
  reference = analogRead(POT1) * ((PI_RAD / 2.0f) / 1023.0f);
#else
  const uint32_t section = sampleIndex / SECTION_SAMPLES;
  if (section >= sizeof(references) / sizeof(references[0])) {
    stopExperiment(false, F("Trajectory finished."));
    return;
  }
  reference = references[section];
#endif
  const float angle = readAngleRad();
  if (!isfinite(angle) || fabsf(angle) > 170.0f * PI_RAD / 180.0f) {
    stopExperiment(true, F("Angle limit exceeded."));
    return;
  }
  const float speed = (angle - previousAngle) / (SAMPLE_US * 1.0e-6f);
  previousAngle = angle;
  // Original discrete LQI gains, with a voltage limit matching FBL.
  // Verify/reidentify the model for the new actuator before tuning gains.
  const float requested = 0.0376f * integralState - 0.3523f * (angle - reference) - 0.1337f * speed;
  const float voltage = constrain(requested, 0.0f, 3.0f);
  // Conditional integration prevents further windup while saturated.
  const float error = reference - angle;
  if (requested == voltage || (requested > 3.0f && error < 0) || (requested < 0 && error > 0)) {
    integralState += error;
  }
  Aero.writeInput(voltage);
  if (sampleIndex % 2 == 0) {
    Serial.print(reference, 3);
    Serial.print(',');
    Serial.print(angle, 3);
    Serial.print(',');
    Serial.println(voltage, 3);
  }
}
