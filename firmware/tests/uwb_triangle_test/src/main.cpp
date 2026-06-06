#include <Arduino.h>
#include <math.h>

bool solveTriangle(float dM1, float dM2, float d12, float& x, float& y) {
  if (dM1 <= 0.05f || dM2 <= 0.05f || d12 <= 0.05f) {
    return false;
  }
  if ((dM1 + dM2 <= d12) || (dM1 + d12 <= dM2) || (dM2 + d12 <= dM1)) {
    return false;
  }

  x = ((dM2 * dM2) + (dM1 * dM1) - (d12 * d12)) / (2.0f * dM1);
  const float ySquared = (dM2 * dM2) - (x * x);
  if (ySquared < 0.0f) {
    return false;
  }
  y = sqrtf(ySquared);
  return true;
}

void setup() {
  Serial.begin(115200);
  Serial.println("UWB triangle solver test");
}

void loop() {
  float x = 0.0f;
  float y = 0.0f;
  const bool valid = solveTriangle(3.0f, 4.0f, 5.0f, x, y);

  Serial.print("valid=");
  Serial.print(valid);
  Serial.print(" slave2=(");
  Serial.print(x, 2);
  Serial.print(", ");
  Serial.print(y, 2);
  Serial.println(")");

  delay(1000);
}
