#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticsState.h"

class Gy511Sensor {
 public:
  bool begin(TwoWire& wire);
  void sample(Gy511DiagnosticState& state);

 private:
  static constexpr uint8_t kAccelAddress = 0x19;
  static constexpr uint8_t kMagAddress = 0x1E;

  TwoWire* wire_ = nullptr;

  bool writeRegister(uint8_t address, uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t address, uint8_t reg, uint8_t* buffer,
                     size_t length);
};

