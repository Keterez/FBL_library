#include "Aero.h"

bool test = 0;  //0 test for sensor
                //1 test for motor


void setup() {
  Serial.begin(9600);
  Aero.begin();
  Aero.calibrate();
}



void loop() {
  if (test) {
    Aero.writeInput(2);
  } else if (!test) {
    Serial.println(Aero.readSensor());
    delay(500);
  }
}