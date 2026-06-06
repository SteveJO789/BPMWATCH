#include "DisplayUI.h"

void DisplayUI::begin() {
  // TODO: Initialize ST7789 after confirming pins.
  Serial.println("Display placeholder ready");
}

void DisplayUI::draw(const TeamMapPacket& packet) {
  Serial.print("Screen map valid: ");
  Serial.print(packet.mapValid);
  Serial.print(" | S1 BPM ");
  Serial.print(packet.slave1Bpm);
  Serial.print(" | S2 BPM ");
  Serial.println(packet.slave2Bpm);

  // TODO: Draw Master, Slave 1, and Slave 2 on ST7789.
}
