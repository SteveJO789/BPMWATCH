#pragma once

#include "Packets.h"

class TeamBroadcaster {
 public:
  void begin();
  void broadcast(const TeamMapPacket& packet);
};
