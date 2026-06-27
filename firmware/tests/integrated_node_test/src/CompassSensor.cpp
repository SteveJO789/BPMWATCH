#include "FeatureFlags.h"

#if BPMWATCH_ENABLE_COMPASS
#include "CompassSensor.h"

#include <Wire.h>

namespace {
constexpr uint8_t kLsm303dWhoAmIReg = 0x0F;
constexpr uint8_t kLsm303dWhoAmIValue = 0x49;
constexpr uint8_t kLsm303dCtrl1 = 0x20;
constexpr uint8_t kLsm303dCtrl2 = 0x21;
constexpr uint8_t kLsm303dCtrl3 = 0x22;
constexpr uint8_t kLsm303dCtrl4 = 0x23;
constexpr uint8_t kLsm303dCtrl5 = 0x24;
constexpr uint8_t kLsm303dCtrl6 = 0x25;
constexpr uint8_t kLsm303dCtrl7 = 0x26;
constexpr uint8_t kLsm303dMagStatus = 0x07;
constexpr uint8_t kLsm303dMagDataStart = 0x08;
constexpr uint8_t kLsm303dAccStatus = 0x27;
constexpr uint8_t kLsm303dAccDataStart = 0x28;

int16_t le16(uint8_t low, uint8_t high) {
  return static_cast<int16_t>((static_cast<uint16_t>(high) << 8) | low);
}

int16_t be16(uint8_t high, uint8_t low) {
  return static_cast<int16_t>((static_cast<uint16_t>(high) << 8) | low);
}

void updateMinMax(CompassDiagnosticState& state) {
  if (state.magX < state.magMinX) state.magMinX = state.magX;
  if (state.magY < state.magMinY) state.magMinY = state.magY;
  if (state.magZ < state.magMinZ) state.magMinZ = state.magZ;
  if (state.magX > state.magMaxX) state.magMaxX = state.magX;
  if (state.magY > state.magMaxY) state.magMaxY = state.magY;
  if (state.magZ > state.magMaxZ) state.magMaxZ = state.magZ;
}

void clearReadState(CompassDiagnosticState& state) {
  state.readOk = false;
  state.magReadOk = false;
  state.accelReadOk = false;
  state.magDataOk = false;
  state.accelDataOk = false;
  state.headingDeg = 0.0f;
}

void setHeadingMode(CompassDiagnosticState& state) {
#if BPMWATCH_ENABLE_TILT_COMPENSATION
  if (state.accelAvailable && state.accelReadOk && state.accelDataOk) {
    state.headingMode = CompassHeadingMode::TiltCompensated;
    return;
  }
#endif
  state.headingMode =
      state.magAvailable ? CompassHeadingMode::MagOnly : CompassHeadingMode::Off;
}

void finalizeCompassMode(CompassDiagnosticState& state) {
  if (state.magDriver == CompassMagDriver::Lsm303d) {
    state.compassMode = state.accelAvailable ? CompassMode::Lsm303d6Dof
                                             : CompassMode::MagOnly;
    return;
  }
  if (state.magAvailable && state.accelAvailable) {
    state.compassMode = CompassMode::Split6Dof;
  } else if (state.magAvailable) {
    state.compassMode = CompassMode::MagOnly;
  } else {
    state.compassMode = CompassMode::Off;
  }
}
}  // namespace

bool CompassSensor::begin(TwoWire& wire) {
  return begin(wire, state_);
}

bool CompassSensor::begin(TwoWire& wire, CompassDiagnosticState& state) {
  wire_ = &wire;
  state = CompassDiagnosticState{};
  state.calibration = calibration_;
  clearReadState(state);
  state_ = state;

  if (detectLsm303d(state)) {
    const bool initOk = initLsm303d(state) && verifyLsm303dConfig(state);
    state.magAvailable = initOk;
    state.accelAvailable = initOk;
    state.magAddress = initOk ? kGy511MagAddress : 0;
    state.accelAddress = initOk ? kGy511MagAddress : 0;
    state.initialized = initOk;
    state.magDriver = initOk ? CompassMagDriver::Lsm303d : CompassMagDriver::None;
    state.accelDriver =
        initOk ? CompassAccelDriver::Lsm303dInternal : CompassAccelDriver::None;
    state.status = initOk ? Gy511Status::Ok : Gy511Status::InitError;
    finalizeCompassMode(state);
    setHeadingMode(state);
    state.magReadOk = state.magAvailable;
    state.accelReadOk = state.accelAvailable;
    if (initOk) {
      state.lastIssue = CompassIssue::None;
      state.lastFailStage = CompassFailStage::None;
    }
    state.i2cReadFailCount += state_.i2cReadFailCount;
    state.i2cWriteFailCount += state_.i2cWriteFailCount;
    state_ = state;
    return state.initialized;
  }

  readMagIdentity(state);
  const bool accelOk = initAccelAt(kGy511PrimaryAccelAddress) ||
                       initAccelAt(kGy511FallbackAccelAddress);
  bool magOk = false;
  if (state.magRegIdA == 0x48 && state.magRegIdB == 0x34 &&
      state.magRegIdC == 0x33) {
    magOk = initMag();
  } else if (state.magAddress == kGy511MagAddress) {
    state.magDriver = CompassMagDriver::Unknown0x1E;
  }

  state.magAvailable = magOk;
  state.accelAvailable = accelOk;
  state.magAddress = magOk ? kGy511MagAddress : state.magAddress;
  state.accelAddress = accelOk ? accelAddress_ : 0;
  state.initialized = magOk;
  state.status = (magOk && accelOk)    ? Gy511Status::Ok
                 : (!magOk && !accelOk) ? Gy511Status::I2cInitError
                 : (!magOk)            ? Gy511Status::MagInitError
                                       : Gy511Status::MagOnly;
  state.magDriver =
      magOk ? CompassMagDriver::HmcCompatible0x1E : state.magDriver;
  state.accelDriver =
      accelOk ? CompassAccelDriver::Lsm303dlhcSplit : CompassAccelDriver::None;
  finalizeCompassMode(state);
  setHeadingMode(state);
  state.magReadOk = state.magAvailable;
  state.accelReadOk = state.accelAvailable;
  if (state.initialized) {
    state.lastIssue = CompassIssue::None;
    state.lastFailStage = CompassFailStage::None;
  }
  state.i2cReadFailCount += state_.i2cReadFailCount;
  state.i2cWriteFailCount += state_.i2cWriteFailCount;
  state_ = state;
  return state.initialized;
}

void CompassSensor::sample(uint32_t nowMs) {
  sample(nowMs, state_);
}

void CompassSensor::sample(uint32_t nowMs, CompassDiagnosticState& state) {
  updateCompassAge(state, nowMs);
  if (wire_ == nullptr || !state.initialized) {
    clearReadState(state);
    if (!gy511StatusIsInitFailure(state.status)) {
      state.status = Gy511Status::InitError;
    }
    state.lastIssue = CompassIssue::Init;
    state_ = state;
    return;
  }

  uint8_t mag[6]{};
  uint8_t accel[6]{};
  uint8_t magStatus = 0;
  uint8_t accelStatus = 0;
  bool magOk = false;
  bool accelOk = !state.accelAvailable;

  if (compassMagDriver(state) == CompassMagDriver::Lsm303d) {
    state.lastFailStage = CompassFailStage::ReadMag;
    if (readRegisters(kGy511MagAddress, kLsm303dMagStatus, &magStatus,
                      sizeof(magStatus)) &&
        readRegisters(kGy511MagAddress, kLsm303dMagDataStart | 0x80, mag,
                      sizeof(mag))) {
      magOk = true;
      state.magRegStatus = magStatus;
      updateMagRegisterDump(state, mag, sizeof(mag));
    } else {
      ++state.i2cReadFailCount;
    }

    if (state.accelAvailable) {
      state.lastFailStage = CompassFailStage::ReadAcc;
      if (readRegisters(kGy511MagAddress, kLsm303dAccStatus, &accelStatus,
                        sizeof(accelStatus)) &&
          readRegisters(kGy511MagAddress, kLsm303dAccDataStart | 0x80, accel,
                        sizeof(accel))) {
        accelOk = true;
        state.accelRegStatus = accelStatus;
        updateAccelRegisterDump(state, accel, sizeof(accel));
      } else {
        ++state.i2cReadFailCount;
        accelOk = false;
      }
    }
  } else {
    if (state.accelAvailable) {
      state.lastFailStage = CompassFailStage::ReadAcc;
      if (readRegisters(accelAddress_, 0x28 | 0x80, accel, sizeof(accel))) {
        accelOk = true;
        updateAccelRegisterDump(state, accel, sizeof(accel));
      } else {
        ++state.i2cReadFailCount;
        accelOk = false;
      }
    }

    state.lastFailStage = CompassFailStage::ReadMag;
    if (readRegisters(kGy511MagAddress, 0x03, mag, sizeof(mag))) {
      magOk = true;
      updateMagRegisterDump(state, mag, sizeof(mag));
    } else {
      ++state.i2cReadFailCount;
    }
  }

  state.magReadOk = magOk;
  state.accelReadOk = accelOk;
  if (!magOk) {
    clearReadState(state);
    state.status = accelOk ? Gy511Status::MagReadError : Gy511Status::I2cReadError;
    state.lastIssue = accelOk ? CompassIssue::MagReadError
                              : CompassIssue::I2cReadError;
    state_ = state;
    return;
  }

  if (compassMagDriver(state) == CompassMagDriver::Lsm303d) {
    state.magX = le16(mag[0], mag[1]);
    state.magY = le16(mag[2], mag[3]);
    state.magZ = le16(mag[4], mag[5]);
  } else {
    state.magX = be16(mag[0], mag[1]);
    state.magZ = be16(mag[2], mag[3]);
    state.magY = be16(mag[4], mag[5]);
  }

  if (accelOk && state.accelAvailable) {
    if (compassMagDriver(state) == CompassMagDriver::Lsm303d) {
      state.accelX = le16(accel[0], accel[1]);
      state.accelY = le16(accel[2], accel[3]);
      state.accelZ = le16(accel[4], accel[5]);
    } else {
      state.accelX = le16(accel[0], accel[1]) >> 4;
      state.accelY = le16(accel[2], accel[3]) >> 4;
      state.accelZ = le16(accel[4], accel[5]) >> 4;
    }
  } else {
    state.accelX = 0;
    state.accelY = 0;
    state.accelZ = 0;
  }

  state.accAbs = static_cast<uint32_t>(abs(state.accelX)) +
                 static_cast<uint32_t>(abs(state.accelY)) +
                 static_cast<uint32_t>(abs(state.accelZ));
  state.accelDataOk = accelOk && state.accAbs > 1000U;

  state.readOk = true;
  if (!updateCompassMagHealth(state)) {
    state_ = state;
    return;
  }

#if BPMWATCH_ENABLE_TILT_COMPENSATION
  if (state.accelDataOk) {
    state.headingDeg = headingFromTiltCompensatedRaw(state);
  } else {
    state.headingDeg =
        compassHeadingFromRaw(state.magX, state.magY, state.calibration);
  }
#else
  state.headingDeg =
      compassHeadingFromRaw(state.magX, state.magY, state.calibration);
#endif

  setHeadingMode(state);
  finalizeCompassMode(state);
  state.readOk = isfinite(state.headingDeg) && state.headingDeg >= 0.0f &&
                 state.headingDeg < 360.0f;
  if (!state.readOk) {
    state.status = Gy511Status::MagReadError;
    state.lastIssue = CompassIssue::Lsm303dMagRawInvalid;
    state_ = state;
    return;
  }

  state.status = (state.accelAvailable && (!state.accelReadOk || !state.accelDataOk))
                     ? Gy511Status::MagOnly
                     : Gy511Status::Ok;
  state.lastIssue = CompassIssue::None;
  state.lastFailStage = CompassFailStage::None;
  ++state.sampleCount;
  state.lastUpdateMs = nowMs;
  updateCompassAge(state, nowMs);
  updateMinMax(state);
  state_ = state;
}

bool CompassSensor::detectLsm303d(CompassDiagnosticState& state) {
#if !BPMWATCH_ENABLE_LSM303D
  (void)state;
  return false;
#else
  state.magAddress = kGy511MagAddress;
  state.lastFailStage = CompassFailStage::DetectWhoAmI;
  uint8_t whoAmI = 0;
  if (!readRegisters(kGy511MagAddress, kLsm303dWhoAmIReg, &whoAmI,
                     sizeof(whoAmI))) {
    ++state.i2cReadFailCount;
    state.lastIssue = CompassIssue::WhoAmIReadError;
    return false;
  }
  state.magWhoAmI = whoAmI;
  return whoAmI == kLsm303dWhoAmIValue;
#endif
}

bool CompassSensor::initAccelAt(uint8_t address) {
  wire_->beginTransmission(address);
  if (wire_->endTransmission() != 0) {
    return false;
  }
  const bool ctrl1Ok = writeRegister(address, 0x20, 0x57);
  const bool ctrl4Ok = ctrl1Ok && writeRegister(address, 0x23, 0x00);
  const bool ok = ctrl1Ok && ctrl4Ok;
  if (!ok) {
    ++state_.i2cWriteFailCount;
  }
  if (ok) {
    accelAddress_ = address;
  }
  return ok;
}

bool CompassSensor::initMag() {
  wire_->beginTransmission(kGy511MagAddress);
  if (wire_->endTransmission() != 0) {
    return false;
  }
  const bool configAOk = writeRegister(kGy511MagAddress, 0x00, 0x14);
  const bool configBOk =
      configAOk && writeRegister(kGy511MagAddress, 0x01, 0x20);
  const bool modeOk = configBOk && writeRegister(kGy511MagAddress, 0x02, 0x00);
  const bool ok = configAOk && configBOk && modeOk;
  if (!ok) {
    ++state_.i2cWriteFailCount;
  }
  return ok;
}

bool CompassSensor::initLsm303d(CompassDiagnosticState& state) {
  wire_->beginTransmission(kGy511MagAddress);
  if (wire_->endTransmission() != 0) {
    ++state.i2cReadFailCount;
    state.lastIssue = CompassIssue::I2cReadError;
    state.lastFailStage = CompassFailStage::InitLsm303d;
    return false;
  }

  state.lastFailStage = CompassFailStage::InitLsm303d;
  if (!writeRegister(kGy511MagAddress, kLsm303dCtrl1, 0x57) ||
      !writeRegister(kGy511MagAddress, kLsm303dCtrl2, 0x00) ||
      !writeRegister(kGy511MagAddress, kLsm303dCtrl4, 0x00) ||
      !writeRegister(kGy511MagAddress, kLsm303dCtrl5, 0x70) ||
      !writeRegister(kGy511MagAddress, kLsm303dCtrl6, 0x20) ||
      !writeRegister(kGy511MagAddress, kLsm303dCtrl7, 0x00)) {
    ++state.i2cWriteFailCount;
    state.lastIssue = CompassIssue::I2cWriteError;
    return false;
  }
  return true;
}

bool CompassSensor::verifyLsm303dConfig(CompassDiagnosticState& state) {
  state.lastFailStage = CompassFailStage::VerifyLsm303d;
  uint8_t ctrl[7]{};
  if (!readRegisters(kGy511MagAddress, kLsm303dCtrl1 | 0x80, ctrl,
                     sizeof(ctrl))) {
    ++state.i2cReadFailCount;
    state.lastIssue = CompassIssue::I2cReadError;
    return false;
  }
  state.lsm303dCtrl20 = ctrl[0];
  state.lsm303dCtrl21 = ctrl[1];
  state.lsm303dCtrl23 = ctrl[3];
  state.lsm303dCtrl24 = ctrl[4];
  state.lsm303dCtrl25 = ctrl[5];
  state.lsm303dCtrl26 = ctrl[6];
  return ctrl[0] == 0x57 && ctrl[1] == 0x00 && ctrl[3] == 0x00 &&
         ctrl[4] == 0x70 && ctrl[5] == 0x20 && ctrl[6] == 0x00;
}

void CompassSensor::readMagIdentity(CompassDiagnosticState& state) {
  if (state.magWhoAmI == kLsm303dWhoAmIValue) {
    return;
  }
  uint8_t id[3]{};
  state.lastFailStage = CompassFailStage::DetectLegacyMagId;
  if (!readRegisters(kGy511MagAddress, 0x0A, id, sizeof(id))) {
    ++state.i2cReadFailCount;
    return;
  }
  state.magAddress = kGy511MagAddress;
  state.magRegIdA = id[0];
  state.magRegIdB = id[1];
  state.magRegIdC = id[2];
}

void CompassSensor::updateMagRegisterDump(CompassDiagnosticState& state,
                                          const uint8_t* magData,
                                          size_t length) {
  for (size_t i = 0; i < sizeof(state.magRegData); ++i) {
    state.magRegData[i] = i < length ? magData[i] : 0;
  }

  if (compassMagDriver(state) == CompassMagDriver::Lsm303d) {
    return;
  }

  uint8_t status = 0;
  if (readRegisters(kGy511MagAddress, 0x09, &status, sizeof(status))) {
    state.magRegStatus = status;
  }

  if (state.magRegIdA == 0 && state.magRegIdB == 0 && state.magRegIdC == 0) {
    readMagIdentity(state);
  }
}

void CompassSensor::updateAccelRegisterDump(CompassDiagnosticState& state,
                                            const uint8_t* accelData,
                                            size_t length) {
  for (size_t i = 0; i < sizeof(state.accelRegData); ++i) {
    state.accelRegData[i] = i < length ? accelData[i] : 0;
  }
}

bool CompassSensor::writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  if (wire_ == nullptr) {
    return false;
  }
  wire_->beginTransmission(address);
  wire_->write(reg);
  wire_->write(value);
  return wire_->endTransmission() == 0;
}

bool CompassSensor::readRegisters(uint8_t address, uint8_t reg, uint8_t* buffer,
                                  size_t length) {
  if (wire_ == nullptr) {
    return false;
  }
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
