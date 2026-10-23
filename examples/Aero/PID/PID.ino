/*
  Adapted for the Feedback Lab (FBL) Aero R1 API from
  AutomationShield/examples/AeroShield/AeroShield_PID.
  Original authors: Peter Tibensky.
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

constexpr uint32_t SAMPLE_US = 5000;
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
  Serial.println(F("reference_pct, angle_pct, input_V"));
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

// Original controller uses angle percent (100% = 90 degrees), and output percent.
#define MANUAL 0
constexpr float KP = 1.0f;
constexpr float TI = 0.55f;
constexpr float TD = 0.25f;
constexpr uint32_t SECTION_SAMPLES = 1000UL / (SAMPLE_US / 1000UL);
const float references[] = {55, 33, 85, 42, 68, 20, 45, 29, 19, 53, 33, 75, 25, 90};
float integral = 0.0f;
float previousError = 0.0f;

void step() {
  float reference;
#if MANUAL
  reference = analogRead(POT1) * (100.0f / 1023.0f);
#else
  const uint32_t section = sampleIndex / SECTION_SAMPLES;
  if (section >= sizeof(references) / sizeof(references[0])) {
    stopExperiment(false, F("Trajectory finished."));
    return;
  }
  reference = references[section];
#endif
  const float angle = readAngleRad();
  const float output = angle * (200.0f / PI_RAD);
  if (!isfinite(angle) || fabsf(output) > 100.0f) {
    stopExperiment(true, F("Angle limit exceeded."));
    return;
  }
  const float dt = SAMPLE_US * 1.0e-6f;
  const float error = reference - output;
  integral = constrain(integral + KP * dt / TI * error, 0.0f, 100.0f);
  const float percent = constrain(KP * error + integral + KP * TD / dt * (error - previousError), 0.0f, 100.0f);
  previousError = error;
  const float voltage = percent * 0.03f;
  Aero.writeInput(voltage);
  if (sampleIndex % 4 == 0) {
    Serial.print(reference, 3);
    Serial.print(',');
    Serial.print(output, 3);
    Serial.print(',');
    Serial.println(voltage, 3);
  }
}
