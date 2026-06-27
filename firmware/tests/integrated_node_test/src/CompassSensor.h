#pragma once

#include <limits.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdint.h>

#include "Gy511Addresses.h"
#include "Gy511Status.h"
#include "TimeUtils.h"

class TwoWire;

struct CompassCalibration {
  // TODO: move these hardcoded calibration values to NVS after bench capture.
  float magOffsetX = 0.0f;
  float magOffsetY = 0.0f;
  float magOffsetZ = 0.0f;
  float magScaleX = 1.0f;
  float magScaleY = 1.0f;
  float magScaleZ = 1.0f;
};

enum class CompassIssue : uint8_t {
  Init,
  None,
  LockTimeout,
  AccelReadError,
  MagReadError,
  I2cReadError,
  I2cWriteError,
  WhoAmIReadError,
  MagRawZero,
  Lsm303dMagRawInvalid,
};

enum class CompassMagDriver : uint8_t {
  None,
  Lsm303d,
  HmcCompatible0x1E,
  Unknown0x1E,
};

enum class CompassAccelDriver : uint8_t {
  None,
  Lsm303dInternal,
  Lsm303dlhcSplit,
};

enum class CompassMode : uint8_t {
  Off,
  MagOnly,
  Split6Dof,
  Lsm303d6Dof,
};

enum class CompassHeadingMode : uint8_t {
  Off,
  MagOnly,
  TiltCompensated,
};

enum class CompassFailStage : uint8_t {
  None,
  DetectWhoAmI,
  DetectLegacyMagId,
  InitAccel,
  InitMag,
  InitLsm303d,
  VerifyLsm303d,
  ReadMag,
  ReadAcc,
};

struct CompassDiagnosticState {
  bool initialized = false;
  bool readOk = false;
  bool magAvailable = false;
  bool accelAvailable = false;
  bool magReadOk = false;
  bool accelReadOk = false;
  bool magDataOk = false;
  bool accelDataOk = false;
  uint8_t magAddress = 0;
  uint8_t accelAddress = 0;
  uint8_t magWhoAmI = 0;
  Gy511Status status = Gy511Status::InitError;
  CompassIssue lastIssue = CompassIssue::Init;
  CompassFailStage lastFailStage = CompassFailStage::None;
  CompassMagDriver magDriver = CompassMagDriver::None;
  CompassAccelDriver accelDriver = CompassAccelDriver::None;
  CompassMode compassMode = CompassMode::Off;
  CompassHeadingMode headingMode = CompassHeadingMode::Off;
  int16_t accelX = 0;
  int16_t accelY = 0;
  int16_t accelZ = 0;
  int16_t magX = 0;
  int16_t magY = 0;
  int16_t magZ = 0;
  uint32_t magAbs = 0;
  uint32_t accAbs = 0;
  uint8_t magRegIdA = 0;
  uint8_t magRegIdB = 0;
  uint8_t magRegIdC = 0;
  uint8_t magRegStatus = 0;
  uint8_t accelRegStatus = 0;
  uint8_t magRegData[6]{};
  uint8_t accelRegData[6]{};
  uint8_t lsm303dCtrl20 = 0;
  uint8_t lsm303dCtrl21 = 0;
  uint8_t lsm303dCtrl23 = 0;
  uint8_t lsm303dCtrl24 = 0;
  uint8_t lsm303dCtrl25 = 0;
  uint8_t lsm303dCtrl26 = 0;
  float headingDeg = 0.0f;
  char i2cAddresses[32] = "SCAN";
  char i2cDeviceList[48] = "SCAN";
  CompassCalibration calibration;
  uint32_t sampleCount = 0;
  uint32_t lastUpdateMs = 0;
  uint32_t lastUpdateAgeMs = 0;
  int16_t magMinX = INT16_MAX;
  int16_t magMinY = INT16_MAX;
  int16_t magMinZ = INT16_MAX;
  int16_t magMaxX = INT16_MIN;
  int16_t magMaxY = INT16_MIN;
  int16_t magMaxZ = INT16_MIN;
  uint32_t i2cLockFailCount = 0;
  uint32_t i2cReadFailCount = 0;
  uint32_t i2cWriteFailCount = 0;
  uint32_t compassTaskStackHighWater = 0;
  bool compassTaskCreated = false;
  uint8_t zeroMagReadCount = 0;
};

using Gy511DiagnosticState = CompassDiagnosticState;

inline float compassHeadingFromRaw(int16_t rawX, int16_t rawY,
                                   const CompassCalibration& calibration) {
  const float x =
      (static_cast<float>(rawX) - calibration.magOffsetX) *
      calibration.magScaleX;
  const float y =
      (static_cast<float>(rawY) - calibration.magOffsetY) *
      calibration.magScaleY;
  float heading = atan2f(y, x) * 180.0f / 3.14159265358979323846f;
  while (heading < 0.0f) {
    heading += 360.0f;
  }
  while (heading >= 360.0f) {
    heading -= 360.0f;
  }
  return heading;
}

inline void updateCompassAge(CompassDiagnosticState& state, uint32_t nowMs) {
  state.lastUpdateAgeMs = safeAgeMs(nowMs, state.lastUpdateMs);
}

inline bool compassHeadingValid(const CompassDiagnosticState& state) {
  return state.initialized && state.magAvailable && state.readOk &&
         state.magReadOk && state.magDataOk && isfinite(state.headingDeg) &&
         state.headingDeg >= 0.0f && state.headingDeg < 360.0f;
}

inline bool compassMagRawAllZero(const CompassDiagnosticState& state) {
  return state.magX == 0 && state.magY == 0 && state.magZ == 0;
}

inline bool updateCompassMagHealth(CompassDiagnosticState& state) {
  state.magAbs = static_cast<uint32_t>(abs(state.magX)) +
                 static_cast<uint32_t>(abs(state.magY)) +
                 static_cast<uint32_t>(abs(state.magZ));
  if (compassMagRawAllZero(state)) {
    state.magDataOk = false;
    state.lastIssue = state.magDriver == CompassMagDriver::Lsm303d
                          ? CompassIssue::Lsm303dMagRawInvalid
                          : CompassIssue::MagRawZero;
    if (state.zeroMagReadCount < 255) {
      ++state.zeroMagReadCount;
    }
    if (state.zeroMagReadCount >= 3) {
      state.readOk = false;
      state.status = Gy511Status::MagReadError;
      return false;
    }
    return true;
  }
  state.zeroMagReadCount = 0;
  state.magDataOk = state.magAbs > 50U;
  if (!state.magDataOk) {
    state.readOk = false;
    state.status = Gy511Status::MagReadError;
    state.lastIssue = CompassIssue::Lsm303dMagRawInvalid;
    return false;
  }
  state.lastIssue = CompassIssue::None;
  return true;
}

inline const char* compassHealthLabel(const CompassDiagnosticState& state) {
  if (!compassHeadingValid(state)) {
    return "FAIL";
  }
  if (!state.accelAvailable || !state.accelReadOk || !state.accelDataOk) {
    return "PARTIAL";
  }
  return "OK";
}

inline const char* compassCompactStatusLabel(
    const CompassDiagnosticState& state) {
  return compassHeadingValid(state) ? "OK" : "ERR";
}

inline const char* compassMagLabel(const CompassDiagnosticState& state) {
  return (state.magAvailable && state.magReadOk) ? "OK" : "FAIL";
}

inline const char* compassAccelLabel(const CompassDiagnosticState& state) {
  if (!state.accelAvailable) {
    return "NONE";
  }
  return state.accelReadOk ? "OK" : "FAIL";
}

inline const char* compassMagAddressLabel(const CompassDiagnosticState& state) {
  return state.magAvailable ? "0x1E" : "NONE";
}

inline const char* compassAccelAddressLabel(
    const CompassDiagnosticState& state) {
  if (!state.accelAvailable) {
    return "NONE";
  }
  if (state.accelAddress == kGy511MagAddress) {
    return "0x1E";
  }
  if (state.accelAddress == kGy511PrimaryAccelAddress) {
    return "0x19";
  }
  if (state.accelAddress == kGy511FallbackAccelAddress) {
    return "0x18";
  }
  return "UNKNOWN";
}

inline const char* compassModeLabel(const CompassDiagnosticState& state) {
  switch (state.compassMode) {
    case CompassMode::Off:
      return "OFF";
    case CompassMode::MagOnly:
      return "MAG_ONLY";
    case CompassMode::Split6Dof:
      return "SPLIT_6DOF";
    case CompassMode::Lsm303d6Dof:
      return "LSM303D_6DOF";
  }
  return "OFF";
}

inline const char* compassHeadingModeLabel(
    const CompassDiagnosticState& state) {
  switch (state.headingMode) {
    case CompassHeadingMode::Off:
      return "OFF";
    case CompassHeadingMode::MagOnly:
      return "MAG_ONLY";
    case CompassHeadingMode::TiltCompensated:
      return "TILT_COMPENSATED";
  }
  return "OFF";
}

inline CompassMagDriver compassMagDriver(const CompassDiagnosticState& state) {
  if (state.magDriver != CompassMagDriver::None) {
    return state.magDriver;
  }
  if (!state.magAvailable) {
    return CompassMagDriver::None;
  }
  if (state.magWhoAmI == 0x49) {
    return CompassMagDriver::Lsm303d;
  }
  if (state.magRegIdA == 0x48 && state.magRegIdB == 0x34 &&
      state.magRegIdC == 0x33) {
    return CompassMagDriver::HmcCompatible0x1E;
  }
  return CompassMagDriver::Unknown0x1E;
}

inline const char* compassMagDriverLabel(const CompassDiagnosticState& state) {
  switch (compassMagDriver(state)) {
    case CompassMagDriver::None:
      return "NONE";
    case CompassMagDriver::Lsm303d:
      return "LSM303D";
    case CompassMagDriver::HmcCompatible0x1E:
      return "HMC_0x1E";
    case CompassMagDriver::Unknown0x1E:
      return "UNKNOWN_0x1E";
  }
  return "UNKNOWN";
}

inline const char* compassAccelDriverLabel(const CompassDiagnosticState& state) {
  switch (state.accelDriver) {
    case CompassAccelDriver::None:
      return "NONE";
    case CompassAccelDriver::Lsm303dInternal:
      return "LSM303D_INTERNAL";
    case CompassAccelDriver::Lsm303dlhcSplit:
      return "LSM303DLHC_SPLIT";
  }
  return "UNKNOWN";
}

inline const char* compassIssueLabel(const CompassDiagnosticState& state) {
  switch (state.lastIssue) {
    case CompassIssue::Init:
      return "INIT";
    case CompassIssue::None:
      return "OK";
    case CompassIssue::LockTimeout:
      return "LOCK_TIMEOUT";
    case CompassIssue::AccelReadError:
      return "ACC_READ_ERR";
    case CompassIssue::MagReadError:
      return "MAG_READ_ERR";
    case CompassIssue::I2cReadError:
      return "I2C_READ_ERR";
    case CompassIssue::I2cWriteError:
      return "I2C_WRITE_ERR";
    case CompassIssue::WhoAmIReadError:
      return "WHOAMI_READ_ERR";
    case CompassIssue::MagRawZero:
      return "MAG_RAW_ZERO";
    case CompassIssue::Lsm303dMagRawInvalid:
      return "LSM303D_MAG_RAW_INVALID";
  }
  return "UNKNOWN";
}

inline const char* compassFailStageLabel(const CompassDiagnosticState& state) {
  switch (state.lastFailStage) {
    case CompassFailStage::None:
      return "NONE";
    case CompassFailStage::DetectWhoAmI:
      return "DETECT_WHOAMI";
    case CompassFailStage::DetectLegacyMagId:
      return "DETECT_LEGACY_ID";
    case CompassFailStage::InitAccel:
      return "INIT_ACCEL";
    case CompassFailStage::InitMag:
      return "INIT_MAG";
    case CompassFailStage::InitLsm303d:
      return "INIT_LSM303D";
    case CompassFailStage::VerifyLsm303d:
      return "VERIFY_LSM303D";
    case CompassFailStage::ReadMag:
      return "READ_MAG";
    case CompassFailStage::ReadAcc:
      return "READ_ACC";
  }
  return "UNKNOWN";
}

inline const char* compassCalibrationStatusLabel(
    const CompassDiagnosticState&) {
  return "HARDCODED";
}

class CompassSensor {
 public:
  bool begin(TwoWire& wire);
  bool begin(TwoWire& wire, CompassDiagnosticState& state);
  void sample(uint32_t nowMs);
  void sample(uint32_t nowMs, CompassDiagnosticState& state);

  bool isValid() const { return compassHeadingValid(state_); }
  float headingDeg() const { return state_.headingDeg; }
  int16_t rawX() const { return state_.magX; }
  int16_t rawY() const { return state_.magY; }
  int16_t rawZ() const { return state_.magZ; }
  const CompassDiagnosticState& state() const { return state_; }
  void setCalibration(const CompassCalibration& calibration) {
    calibration_ = calibration;
    state_.calibration = calibration;
  }

 private:
  TwoWire* wire_ = nullptr;
  uint8_t accelAddress_ = kGy511PrimaryAccelAddress;
  CompassCalibration calibration_;
  CompassDiagnosticState state_;

  bool detectLsm303d(CompassDiagnosticState& state);
  bool initAccelAt(uint8_t address);
  bool initMag();
  bool initLsm303d(CompassDiagnosticState& state);
  bool verifyLsm303dConfig(CompassDiagnosticState& state);
  void readMagIdentity(CompassDiagnosticState& state);
  void updateMagRegisterDump(CompassDiagnosticState& state,
                             const uint8_t* magData, size_t length);
  void updateAccelRegisterDump(CompassDiagnosticState& state,
                               const uint8_t* accelData, size_t length);
  bool writeRegister(uint8_t address, uint8_t reg, uint8_t value);
  bool readRegisters(uint8_t address, uint8_t reg, uint8_t* buffer,
                     size_t length);
};
