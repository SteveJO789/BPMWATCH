#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  WiFi.mode(WIFI_STA);

  Serial.println("ESP32 MAC address scanner");
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
}

void loop() {
  delay(1000);
}
