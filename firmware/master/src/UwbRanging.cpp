#include "UwbRanging.h"

#include <DW1000.h>
#include <SPI.h>

namespace {
constexpr int kUwbSck = 18;
constexpr int kUwbMiso = 19;
constexpr int kUwbMosi = 23;
constexpr int kUwbCs = 5;
constexpr int kUwbIrq = 34;
constexpr int kUwbRst = 4;
}

void UwbRanging::begin() {
  Serial.println("B&T BU01 DW1000 using default SPI");
  SPI.begin(kUwbSck, kUwbMiso, kUwbMosi, kUwbCs);
  DW1000.begin(kUwbIrq, kUwbRst);
  DW1000.select(kUwbCs);
  // TODO: Replace mock distances with real DW1000 ranging exchange.
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
