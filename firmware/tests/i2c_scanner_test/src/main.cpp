#include <Arduino.h>
#include <Wire.h>

constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;

void setup() {
  Serial.begin(115200);
  delay(500);
  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("BPMWATCH I2C scanner");
  Serial.println("Expected common addresses:");
  Serial.println("- GY-511 / LSM303DLHC accel: 0x19");
  Serial.println("- GY-511 / LSM303DLHC magnetometer: 0x1E");
  Serial.println("- MAX30102: 0x57");
}

void loop() {
  int found = 0;
  Serial.println("Scanning I2C bus...");

  for (uint8_t address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    const uint8_t error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("Found I2C device at 0x");
      if (address < 16) {
        Serial.print("0");
      }
      Serial.println(address, HEX);
      found++;
    }
  }

  if (found == 0) {
    Serial.println("No I2C devices found. Check SDA/SCL, 3.3V, GND, and pullups.");
  } else {
    Serial.print("Device count: ");
    Serial.println(found);
  }

  delay(3000);
}
