#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticsDisplay.h"
#include "DiagnosticsState.h"
#include "Gy511Sensor.h"
#include "Max30102Sensor.h"
#include "NodeConfig.h"
#include "UwbDiagnostics.h"

namespace {
DiagnosticsState state;
DiagnosticsDisplay display;
Gy511Sensor gy511;
Max30102Sensor max30102;
UwbDiagnostics uwb;

uint32_t lastMaxMs = 0;
uint32_t lastGyMs = 0;
uint32_t lastDisplayMs = 0;
uint32_t lastLogMs = 0;

void logDiagnostics(const DiagnosticsState& current, uint32_t nowMs) {
  const char* uwbStatus = !current.uwb.spiReady
                              ? "SPI_ERR"
                          : current.uwb.rangeStale ? "LOST"
                          : current.uwb.hasRange   ? "OK"
                                                   : "WAIT";
  const char* gyStatus = current.gy511.initialized && current.gy511.readOk
                             ? "OK"
                             : "ERR";
  const char* maxStatus = !current.max30102.initialized
                              ? "ERR"
                          : current.max30102.fingerPresent ? "OK"
                                                           : "NO_FINGER";

  Serial.printf(
      "t=%lu %s | UWB=%s D=%.2fm | GY=%s H=%.1f A=%d,%d,%d | "
      "MAX=%s IR=%ld BPM=%d\n",
      static_cast<unsigned long>(nowMs), kNodeConfig.displayLabel, uwbStatus,
      current.uwb.distanceM, gyStatus, current.gy511.headingDeg,
      current.gy511.accelX, current.gy511.accelY, current.gy511.accelZ,
      maxStatus, current.max30102.irValue, current.max30102.averageBpm);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.printf("BPMWATCH integrated diagnostics: %s\n",
                kNodeConfig.displayLabel);

  Wire.begin(21, 22);
  display.begin(kNodeConfig.displayLabel);

  state.gy511.initialized = gy511.begin(Wire);
  state.gy511.readOk = state.gy511.initialized;
  Serial.printf("GY-511 init: %s\n",
                state.gy511.initialized ? "OK" : "ERROR");

  state.max30102.initialized = max30102.begin(Wire);
  Serial.printf("MAX30102 init: %s\n",
                state.max30102.initialized ? "OK" : "ERROR");

  uwb.begin(state.uwb);
}

void loop() {
  const uint32_t nowMs = millis();
  uwb.poll(nowMs, state.uwb);

  if (nowMs - lastMaxMs >= 20) {
    lastMaxMs = nowMs;
    max30102.sample(nowMs, state.max30102);
  }

  if (nowMs - lastGyMs >= 50) {
    lastGyMs = nowMs;
    gy511.sample(state.gy511);
  }

  if (nowMs - lastDisplayMs >= 200) {
    lastDisplayMs = nowMs;
    display.render(state, nowMs);
  }

  if (nowMs - lastLogMs >= 1000) {
    lastLogMs = nowMs;
    logDiagnostics(state, nowMs);
  }
}

