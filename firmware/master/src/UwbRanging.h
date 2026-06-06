#pragma once

#include "Packets.h"

class UwbRanging {
 public:
  void begin();
  DistancePacket readPair(uint8_t fromId, uint8_t toId);

 private:
  float mockDistance(uint8_t fromId, uint8_t toId) const;
};
