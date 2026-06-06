#include <Arduino.h>

#include "Packets.h"
#include "RelativeMapSolver.h"
#include "TeamBroadcaster.h"
#include "UwbRanging.h"

namespace {
RelativeMapSolver mapSolver;
TeamBroadcaster broadcaster;
UwbRanging ranging;

SlaveStatusPacket slave1{NODE_SLAVE_1, 82, 95, 1};
SlaveStatusPacket slave2{NODE_SLAVE_2, 91, 92, 1};

uint32_t lastUpdateMs = 0;

int readMasterBatteryPercent() {
  // TODO: Read calibrated battery ADC.
  return 100;
}
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("BPMWATCH master starting");

  ranging.begin();
  broadcaster.begin();
}

void loop() {
  if (millis() - lastUpdateMs < 500) {
    return;
  }
  lastUpdateMs = millis();

  const DistancePacket dM1 = ranging.readPair(NODE_MASTER, NODE_SLAVE_1);
  const DistancePacket dM2 = ranging.readPair(NODE_MASTER, NODE_SLAVE_2);
  const DistancePacket d12 = ranging.readPair(NODE_SLAVE_1, NODE_SLAVE_2);

  const TeamMapPacket map = mapSolver.solve(dM1.distanceMeters, dM2.distanceMeters,
                                            d12.distanceMeters, slave1, slave2,
                                            readMasterBatteryPercent());
  broadcaster.broadcast(map);
}
