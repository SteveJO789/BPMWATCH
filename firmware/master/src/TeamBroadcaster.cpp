#include "TeamBroadcaster.h"

void TeamBroadcaster::begin() {
  // TODO: Initialize ESP-NOW or chosen radio transport and register slave MAC addresses.
}

void TeamBroadcaster::broadcast(const TeamMapPacket& packet) {
  Serial.print("Map valid: ");
  Serial.print(packet.mapValid);
  Serial.print(" | S1(");
  Serial.print(packet.slave1X, 2);
  Serial.print(", ");
  Serial.print(packet.slave1Y, 2);
  Serial.print(") S2(");
  Serial.print(packet.slave2X, 2);
  Serial.print(", ");
  Serial.print(packet.slave2Y, 2);
  Serial.println(")");

  // TODO: Send TeamMapPacket to Slave 1 and Slave 2.
}
