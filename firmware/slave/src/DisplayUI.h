#pragma once

#include "Packets.h"

class DisplayUI {
 public:
  void begin();
  void draw(const TeamMapPacket& packet);
};
