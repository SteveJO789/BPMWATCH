#include <Arduino.h>

constexpr int LED_PIN = 2;

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  Serial.println("ESP32 blink test");
}

void loop() {
  digitalWrite(LED_PIN, HIGH);
  Serial.println("LED on");
  delay(500);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED off");
  delay(500);
}
