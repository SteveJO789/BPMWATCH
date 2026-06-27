#pragma once

#include <Arduino.h>
#include <Wire.h>
#include <MAX30105.h>

#include "DiagnosticsState.h"

class Max30102Sensor {
 public:
  bool begin(TwoWire& wire);
  void sample(uint32_t nowMs, Max30102DiagnosticState& state);

 private:
  static constexpr uint8_t kRateSize = 4;
  static constexpr uint8_t kIntervalBufferSize = 5;

  MAX30105 sensor_;
  uint8_t rates_[kRateSize]{};
  uint8_t rateSpot_ = 0;
  uint8_t validRateCount_ = 0;
  uint16_t intervals_[kIntervalBufferSize]{};
  uint8_t intervalSpot_ = 0;
  uint8_t validIntervalCount_ = 0;
  uint32_t lastRawBeatMs_ = 0;
  uint32_t lastAcceptedBeatMs_ = 0;
  uint32_t lastAcceptedIntervalMs_ = 0;
  uint32_t signalLostSinceMs_ = 0;

  void resetInternalBeatState();
  void clearAcceptedIntervalState(Max30102DiagnosticState& state);
  void clearBeatTimingState(Max30102DiagnosticState& state);
  void resetBeatState(Max30102DiagnosticState& state);
  void reseedAcceptedBeatClock(uint32_t nowMs,
                               Max30102DiagnosticState& state);
  bool intervalJumpAcceptable(uint32_t intervalMs) const;
  uint16_t medianIntervalMs() const;
  void pushAcceptedInterval(uint32_t intervalMs);
  void rejectBeatCandidate(Max30102DiagnosticState& state,
                           bool refractory,
                           bool intervalOutOfRange,
                           bool unstable);
};
