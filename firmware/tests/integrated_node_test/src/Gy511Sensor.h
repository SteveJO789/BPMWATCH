#pragma once

#include <Arduino.h>
#include <Wire.h>

#include "DiagnosticsState.h"
#include "Gy511Addresses.h"

class Gy511Sensor {
 public:
  bool begin(TwoWire& wire);
  bool begin(TwoWire& wire, Gy511DiagnosticState& state);
  void sample(Gy511DiagnosticState& state);

 private:
  TwoWire* wire_ = nullptr;
  uint8_t accelAddress_ = kGy511PrimaryAccelAddress;

  bool initAccelAt(uint8_t address);
  bool initMag();
  bool writeRegister(uint8_t address, uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t address, uint8_t reg, uint8_t* buffer,
                     size_t length);
};
