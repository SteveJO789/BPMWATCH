#include "FeatureFlags.h"

#if BPMWATCH_ENABLE_GY511
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
  Gy511DiagnosticState state;
  return begin(wire, state);
}

bool Gy511Sensor::begin(TwoWire& wire, Gy511DiagnosticState& state) {
  wire_ = &wire;
  bool accelOk = initAccelAt(kGy511PrimaryAccelAddress);
  if (!accelOk) {
    accelOk = initAccelAt(kGy511FallbackAccelAddress);
  }
  const bool magOk = initMag();

  state.accelAvailable = accelOk;
  state.initialized = magOk;
  state.readOk = state.initialized;
  if (accelOk && magOk) {
    state.status = Gy511Status::Ok;
  } else if (!accelOk && !magOk) {
    state.status = Gy511Status::I2cInitError;
  } else if (!accelOk) {
    state.status = Gy511Status::MagOnly;
  } else {
    state.status = Gy511Status::MagInitError;
  }
  return state.initialized;
}

void Gy511Sensor::sample(Gy511DiagnosticState& state) {
  if (wire_ == nullptr || !state.initialized) {
    state.readOk = false;
    if (!gy511StatusIsInitFailure(state.status)) {
      state.status = Gy511Status::InitError;
    }
    return;
  }

  uint8_t accel[6]{};
  uint8_t mag[6]{};
  const bool accelOk = !state.accelAvailable ||
                       readRegisters(accelAddress_, 0x28 | 0x80, accel,
                                     sizeof(accel));
  const bool magOk = readRegisters(kGy511MagAddress, 0x03, mag, sizeof(mag));
  state.readOk = accelOk && magOk;
  if (!state.readOk) {
    if (!accelOk && !magOk) {
      state.status = Gy511Status::I2cReadError;
    } else if (!accelOk) {
      state.status = Gy511Status::AccelReadError;
    } else {
      state.status = Gy511Status::MagReadError;
    }
    return;
  }
  state.status = state.accelAvailable ? Gy511Status::Ok : Gy511Status::MagOnly;

  if (state.accelAvailable) {
    state.accelX = le16(accel[0], accel[1]) >> 4;
    state.accelY = le16(accel[2], accel[3]) >> 4;
    state.accelZ = le16(accel[4], accel[5]) >> 4;
  } else {
    state.accelX = 0;
    state.accelY = 0;
    state.accelZ = 0;
  }
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

bool Gy511Sensor::initAccelAt(uint8_t address) {
  const bool ok = writeRegister(address, 0x20, 0x57) &&
                  writeRegister(address, 0x23, 0x00);
  if (ok) {
    accelAddress_ = address;
  }
  return ok;
}

bool Gy511Sensor::initMag() {
  return writeRegister(kGy511MagAddress, 0x00, 0x14) &&
         writeRegister(kGy511MagAddress, 0x01, 0x20) &&
         writeRegister(kGy511MagAddress, 0x02, 0x00);
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
#endif
