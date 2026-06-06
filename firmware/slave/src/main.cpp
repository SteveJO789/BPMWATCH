#include <Arduino.h>

#include "BatteryMonitor.h"
#include "DisplayUI.h"
#include "HeartRateSensor.h"
#include "Packets.h"
#include "UwbRanging.h"

#ifndef BPMWATCH_NODE_ID
#define BPMWATCH_NODE_ID 1
#endif

namespace {
BatteryMonitor battery;
DisplayUI display;
HeartRateSensor heartRate;
UwbRanging ranging;

TeamMapPacket latestMap{};
uint32_t lastStatusMs = 0;
uint32_t lastDrawMs = 0;

void sendStatusPacket() {
  SlaveStatusPacket status{};
  status.id = BPMWATCH_NODE_ID;
  status.bpm = heartRate.readBpm();
  status.battery = battery.readPercent();
  status.signal = 1;

  Serial.print("Send status node ");
  Serial.print(status.id);
  Serial.print(" BPM ");
  Serial.print(status.bpm);
  Serial.print(" battery ");
  Serial.println(status.battery);

  // TODO: Send SlaveStatusPacket to master.
}

void receiveMockMap() {
  // TODO: Replace with real TeamMapPacket receive callback.
  latestMap.masterX = 0.0f;
  latestMap.masterY = 0.0f;
  latestMap.slave1X = 3.0f;
  latestMap.slave1Y = 0.0f;
  latestMap.slave2X = 3.0f;
  latestMap.slave2Y = 4.0f;
  latestMap.masterBattery = 100;
  latestMap.slave1Bpm = 82;
  latestMap.slave1Battery = 95;
  latestMap.slave2Bpm = 91;
  latestMap.slave2Battery = 92;
  latestMap.mapValid = 1;
}
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("BPMWATCH slave starting");

  battery.begin();
  heartRate.begin();
  display.begin();
  ranging.begin();
}

void loop() {
  ranging.poll();
  receiveMockMap();

  if (millis() - lastStatusMs >= 300) {
    lastStatusMs = millis();
    sendStatusPacket();
  }

  if (millis() - lastDrawMs >= 500) {
    lastDrawMs = millis();
    display.draw(latestMap);
  }
}
