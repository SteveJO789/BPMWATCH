#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  Serial.println("BU01/DW1000 pair ranging test placeholder");
  Serial.println("TODO: Add real UART/SPI protocol after module mode is confirmed.");
}

void loop() {
  static uint32_t lastMs = 0;
  if (millis() - lastMs >= 1000) {
    lastMs = millis();
    Serial.println("Mock pair distance: 2.00 m quality=2");
  }
}
