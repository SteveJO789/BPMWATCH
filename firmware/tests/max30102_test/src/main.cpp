#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"

MAX30105 sensor;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    return;
  }

  sensor.setup();
  Serial.println("MAX30102 raw IR test");
}

void loop() {
  Serial.print("IR=");
  Serial.println(sensor.getIR());
  delay(250);
}
