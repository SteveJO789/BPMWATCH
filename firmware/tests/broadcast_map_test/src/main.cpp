#include <Arduino.h>

typedef struct {
  float masterX;
  float masterY;
  float slave1X;
  float slave1Y;
  float slave2X;
  float slave2Y;
  int masterBattery;
  int slave1Bpm;
  int slave1Battery;
  int slave2Bpm;
  int slave2Battery;
  uint8_t mapValid;
} TeamMapPacket;

void printPacket(const TeamMapPacket& packet) {
  Serial.print("valid=");
  Serial.print(packet.mapValid);
  Serial.print(" S1 BPM=");
  Serial.print(packet.slave1Bpm);
  Serial.print(" S2 BPM=");
  Serial.println(packet.slave2Bpm);
}

void setup() {
  Serial.begin(115200);
  Serial.println("TeamMapPacket broadcast test placeholder");
}

void loop() {
  TeamMapPacket packet{0, 0, 3, 0, 3, 4, 100, 82, 95, 91, 92, 1};
  printPacket(packet);
  // TODO: Send this packet through ESP-NOW after slave MAC addresses are known.
  delay(1000);
}
