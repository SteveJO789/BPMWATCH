#include "HeartRateSensor.h"

#include <Arduino.h>

void HeartRateSensor::begin() {
  // TODO: Initialize MAX30102 and sensor calibration.
}

int HeartRateSensor::readBpm() {
  // TODO: Replace mock value with MAX30102 BPM calculation.
  return 80 + ((millis() / 1000) % 5);
}
