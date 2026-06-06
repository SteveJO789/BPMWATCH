#include "UwbRanging.h"

void UwbRanging::begin() {
  // TODO: Initialize BU01/DW1000 UART or SPI after confirming module mode.
}

DistancePacket UwbRanging::readPair(uint8_t fromId, uint8_t toId) {
  DistancePacket packet{};
  packet.fromId = fromId;
  packet.toId = toId;
  packet.distanceMeters = mockDistance(fromId, toId);
  packet.timestampMs = millis();
  packet.quality = 2;
  return packet;
}

float UwbRanging::mockDistance(uint8_t fromId, uint8_t toId) const {
  if ((fromId == NODE_MASTER && toId == NODE_SLAVE_1) ||
      (fromId == NODE_SLAVE_1 && toId == NODE_MASTER)) {
    return 3.0f;
  }
  if ((fromId == NODE_MASTER && toId == NODE_SLAVE_2) ||
      (fromId == NODE_SLAVE_2 && toId == NODE_MASTER)) {
    return 4.0f;
  }
  return 5.0f;
}
