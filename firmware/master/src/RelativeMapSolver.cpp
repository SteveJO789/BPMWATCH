#include "RelativeMapSolver.h"

#include <math.h>

bool RelativeMapSolver::isValidTriangle(float a, float b, float c) const {
  if (a <= 0.05f || b <= 0.05f || c <= 0.05f) {
    return false;
  }

  return (a + b > c) && (a + c > b) && (b + c > a);
}

TeamMapPacket RelativeMapSolver::solve(float masterToSlave1, float masterToSlave2,
                                       float slave1ToSlave2,
                                       const SlaveStatusPacket& slave1,
                                       const SlaveStatusPacket& slave2,
                                       int masterBattery) const {
  TeamMapPacket packet{};
  packet.masterX = 0.0f;
  packet.masterY = 0.0f;
  packet.slave1X = masterToSlave1;
  packet.slave1Y = 0.0f;
  packet.masterBattery = masterBattery;
  packet.slave1Bpm = slave1.bpm;
  packet.slave1Battery = slave1.battery;
  packet.slave2Bpm = slave2.bpm;
  packet.slave2Battery = slave2.battery;

  if (!isValidTriangle(masterToSlave1, masterToSlave2, slave1ToSlave2)) {
    packet.mapValid = 0;
    return packet;
  }

  const float x = ((masterToSlave2 * masterToSlave2) + (masterToSlave1 * masterToSlave1) -
                   (slave1ToSlave2 * slave1ToSlave2)) /
                  (2.0f * masterToSlave1);
  const float ySquared = (masterToSlave2 * masterToSlave2) - (x * x);

  if (ySquared < -0.01f) {
    packet.mapValid = 0;
    return packet;
  }

  packet.slave2X = x;
  packet.slave2Y = sqrtf(max(0.0f, ySquared));
  packet.mapValid = 1;
  return packet;
}
