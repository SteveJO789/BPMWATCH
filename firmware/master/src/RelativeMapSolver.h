#pragma once

#include "Packets.h"

class RelativeMapSolver {
 public:
  TeamMapPacket solve(float masterToSlave1, float masterToSlave2, float slave1ToSlave2,
                      const SlaveStatusPacket& slave1, const SlaveStatusPacket& slave2,
                      int masterBattery) const;

 private:
  bool isValidTriangle(float a, float b, float c) const;
};
