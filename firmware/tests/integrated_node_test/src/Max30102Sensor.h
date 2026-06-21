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

  MAX30105 sensor_;
  uint8_t rates_[kRateSize]{};
  uint8_t rateSpot_ = 0;
  uint8_t validRateCount_ = 0;
  uint32_t lastBeatMs_ = 0;
};

