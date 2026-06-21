#include "Gy511Sensor.h"

#include <math.h>

namespace {
int16_t le16(uint8_t low, uint8_t high) {
  return static_cast<int16_t>((high << 8) | low);
}

int16_t be16(uint8_t high, uint8_t low) {
  return static_cast<int16_t>((high << 8) | low);
}
}  // namespace

bool Gy511Sensor::begin(TwoWire& wire) {
  wire_ = &wire;
  bool ok = true;
  ok &= writeRegister(kAccelAddress, 0x20, 0x57);
  ok &= writeRegister(kAccelAddress, 0x23, 0x00);
  ok &= writeRegister(kMagAddress, 0x00, 0x14);
  ok &= writeRegister(kMagAddress, 0x01, 0x20);
  ok &= writeRegister(kMagAddress, 0x02, 0x00);
  return ok;
}

void Gy511Sensor::sample(Gy511DiagnosticState& state) {
  if (wire_ == nullptr || !state.initialized) {
    state.readOk = false;
    return;
  }

  uint8_t accel[6]{};
  uint8_t mag[6]{};
  const bool accelOk =
      readRegisters(kAccelAddress, 0x28 | 0x80, accel, sizeof(accel));
  const bool magOk = readRegisters(kMagAddress, 0x03, mag, sizeof(mag));
  state.readOk = accelOk && magOk;
  if (!state.readOk) {
    return;
  }

  state.accelX = le16(accel[0], accel[1]) >> 4;
  state.accelY = le16(accel[2], accel[3]) >> 4;
  state.accelZ = le16(accel[4], accel[5]) >> 4;
  state.magX = be16(mag[0], mag[1]);
  state.magZ = be16(mag[2], mag[3]);
  state.magY = be16(mag[4], mag[5]);

  state.headingDeg =
      atan2f(static_cast<float>(state.magY), static_cast<float>(state.magX)) *
      180.0f / PI;
  if (state.headingDeg < 0.0f) {
    state.headingDeg += 360.0f;
  }
}

bool Gy511Sensor::writeRegister(uint8_t address, uint8_t reg,
                                uint8_t value) {
  wire_->beginTransmission(address);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

bool Gy511Sensor::readRegisters(uint8_t address, uint8_t reg,
                                uint8_t* buffer, size_t length) {
  wire_->beginTransmission(address);
  wire_->write(reg);
  if (wire_->endTransmission(false) != 0) {
    return false;
  }

  const size_t received = wire_->requestFrom(static_cast<int>(address),
                                              static_cast<int>(length));
  if (received != length) {
    return false;
  }
  for (size_t i = 0; i < length; ++i) {
    buffer[i] = wire_->read();
  }
  return true;
}

