#include <FBLSampling.h>

void step() {
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void samplingError() {
  // Sampling already stopped. This callback runs in loop(), not in an ISR.
  digitalWrite(LED_BUILTIN, LOW);
  Serial.println(F("Sampling deadline missed."));
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  // Microseconds: toggle every 100 ms. First call occurs after one period.
  if (!Sampling.begin(100000UL, step, samplingError)) {
    Serial.print(F("Cannot start sampling. Error code: "));
    Serial.println(static_cast<unsigned int>(Sampling.error()));
  }
}

void loop() {
  Sampling.update();
  // Other non-blocking work can run here.
}
