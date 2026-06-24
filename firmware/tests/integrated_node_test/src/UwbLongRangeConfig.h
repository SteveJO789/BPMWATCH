#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

struct UwbLongRangeRegisterSnapshot {
  uint32_t sysCfg = 0;
  uint32_t chanCtrl = 0;
  uint32_t txFctrlLow = 0;
  uint8_t txFctrlHigh = 0;
  uint32_t txPower = 0;
  uint32_t rfTxctrl = 0;
  uint8_t tcPgdelay = 0;
  uint32_t fsPllcfg = 0;
  uint8_t fsPlltune = 0;
  uint16_t agcTune1 = 0;
  uint32_t agcTune2 = 0;
  uint16_t drxTune0b = 0;
  uint16_t drxTune1a = 0;
  uint16_t drxTune1b = 0;
  uint32_t drxTune2 = 0;
  uint16_t drxSfdtoc = 0;
  uint8_t ldeCfg1 = 0;
  uint16_t ldeCfg2 = 0;
  uint16_t ldeRepc = 0;
  uint32_t sysMask = 0;
  uint32_t sysStatusLow = 0;
  uint8_t sysStatusHigh = 0;
};

enum UwbLongRangeFailure : uint32_t {
  UwbLongRangeFailSysCfg = 1UL << 0,
  UwbLongRangeFailChanCtrl = 1UL << 1,
  UwbLongRangeFailTxFctrl = 1UL << 2,
  UwbLongRangeFailTxPower = 1UL << 3,
  UwbLongRangeFailRfTxctrl = 1UL << 4,
  UwbLongRangeFailTcPgdelay = 1UL << 5,
  UwbLongRangeFailFsPllcfg = 1UL << 6,
  UwbLongRangeFailFsPlltune = 1UL << 7,
  UwbLongRangeFailAgcTune1 = 1UL << 8,
  UwbLongRangeFailAgcTune2 = 1UL << 9,
  UwbLongRangeFailDrxTune0b = 1UL << 10,
  UwbLongRangeFailDrxTune1a = 1UL << 11,
  UwbLongRangeFailDrxTune1b = 1UL << 12,
  UwbLongRangeFailDrxTune2 = 1UL << 13,
  UwbLongRangeFailDrxSfdtoc = 1UL << 14,
  UwbLongRangeFailLdeCfg1 = 1UL << 15,
  UwbLongRangeFailLdeCfg2 = 1UL << 16,
  UwbLongRangeFailLdeRepc = 1UL << 17,
  UwbLongRangeFailSysMask = 1UL << 18
};

constexpr uint32_t kExpectedUwbLongRangeSysCfgMask = 1UL << 22;
constexpr uint32_t kExpectedUwbLongRangeSysCfg = 1UL << 22;
constexpr uint32_t kExpectedUwbLongRangeChanCtrl = 0x528A0055UL;
constexpr uint32_t kExpectedUwbLongRangeTxFctrlMask = 0x003F6000UL;
constexpr uint32_t kExpectedUwbLongRangeTxFctrl = 0x002A0000UL;
constexpr uint32_t kExpectedUwbLongRangeTxPower = 0x85858585UL;
constexpr uint32_t kExpectedUwbLongRangeRfTxctrl = 0x001E3FE0UL;
constexpr uint8_t kExpectedUwbLongRangeTcPgdelay = 0xC0;
constexpr uint32_t kExpectedUwbLongRangeFsPllcfg = 0x0800041DUL;
constexpr uint8_t kExpectedUwbLongRangeFsPlltune = 0xBE;
constexpr uint16_t kExpectedUwbLongRangeAgcTune1 = 0x889B;
constexpr uint32_t kExpectedUwbLongRangeAgcTune2 = 0x2502A907UL;
constexpr uint16_t kExpectedUwbLongRangeDrxTune0b = 0x0016;
constexpr uint16_t kExpectedUwbLongRangeDrxTune1a = 0x008D;
constexpr uint16_t kExpectedUwbLongRangeDrxTune1b = 0x0064;
constexpr uint32_t kExpectedUwbLongRangeDrxTune2 = 0x373B0296UL;
constexpr uint16_t kExpectedUwbLongRangeDrxSfdtoc = 0x0801;
constexpr uint8_t kExpectedUwbLongRangeLdeCfg1 = 0x0D;
constexpr uint16_t kExpectedUwbLongRangeLdeCfg2 = 0x0607;
constexpr uint16_t kExpectedUwbLongRangeLdeRepc = 0x0666;
constexpr uint32_t kExpectedUwbLongRangeSysMask =
    (1UL << 3) | (1UL << 7) | (1UL << 12) | (1UL << 13) |
    (1UL << 14) | (1UL << 15) | (1UL << 16) | (1UL << 17) |
    (1UL << 18) | (1UL << 21) | (1UL << 26);
constexpr uint32_t kExpectedUwbLongRangeSysMaskMask =
    kExpectedUwbLongRangeSysMask;

inline UwbLongRangeRegisterSnapshot expectedUwbLongRangeSnapshot() {
  UwbLongRangeRegisterSnapshot snapshot;
  snapshot.sysCfg = kExpectedUwbLongRangeSysCfg;
  snapshot.chanCtrl = kExpectedUwbLongRangeChanCtrl;
  snapshot.txFctrlLow = kExpectedUwbLongRangeTxFctrl;
  snapshot.txPower = kExpectedUwbLongRangeTxPower;
  snapshot.rfTxctrl = kExpectedUwbLongRangeRfTxctrl;
  snapshot.tcPgdelay = kExpectedUwbLongRangeTcPgdelay;
  snapshot.fsPllcfg = kExpectedUwbLongRangeFsPllcfg;
  snapshot.fsPlltune = kExpectedUwbLongRangeFsPlltune;
  snapshot.agcTune1 = kExpectedUwbLongRangeAgcTune1;
  snapshot.agcTune2 = kExpectedUwbLongRangeAgcTune2;
  snapshot.drxTune0b = kExpectedUwbLongRangeDrxTune0b;
  snapshot.drxTune1a = kExpectedUwbLongRangeDrxTune1a;
  snapshot.drxTune1b = kExpectedUwbLongRangeDrxTune1b;
  snapshot.drxTune2 = kExpectedUwbLongRangeDrxTune2;
  snapshot.drxSfdtoc = kExpectedUwbLongRangeDrxSfdtoc;
  snapshot.ldeCfg1 = kExpectedUwbLongRangeLdeCfg1;
  snapshot.ldeCfg2 = kExpectedUwbLongRangeLdeCfg2;
  snapshot.ldeRepc = kExpectedUwbLongRangeLdeRepc;
  snapshot.sysMask = kExpectedUwbLongRangeSysMask;
  return snapshot;
}

inline uint32_t uwbLongRangeFailureMask(
    const UwbLongRangeRegisterSnapshot& snapshot) {
  uint32_t mask = 0;
  if ((snapshot.sysCfg & kExpectedUwbLongRangeSysCfgMask) !=
      kExpectedUwbLongRangeSysCfg) {
    mask |= UwbLongRangeFailSysCfg;
  }
  if (snapshot.chanCtrl != kExpectedUwbLongRangeChanCtrl) {
    mask |= UwbLongRangeFailChanCtrl;
  }
  if ((snapshot.txFctrlLow & kExpectedUwbLongRangeTxFctrlMask) !=
      kExpectedUwbLongRangeTxFctrl) {
    mask |= UwbLongRangeFailTxFctrl;
  }
  if (snapshot.txPower != kExpectedUwbLongRangeTxPower) {
    mask |= UwbLongRangeFailTxPower;
  }
  if (snapshot.rfTxctrl != kExpectedUwbLongRangeRfTxctrl) {
    mask |= UwbLongRangeFailRfTxctrl;
  }
  if (snapshot.tcPgdelay != kExpectedUwbLongRangeTcPgdelay) {
    mask |= UwbLongRangeFailTcPgdelay;
  }
  if (snapshot.fsPllcfg != kExpectedUwbLongRangeFsPllcfg) {
    mask |= UwbLongRangeFailFsPllcfg;
  }
  if (snapshot.fsPlltune != kExpectedUwbLongRangeFsPlltune) {
    mask |= UwbLongRangeFailFsPlltune;
  }
  if (snapshot.agcTune1 != kExpectedUwbLongRangeAgcTune1) {
    mask |= UwbLongRangeFailAgcTune1;
  }
  if (snapshot.agcTune2 != kExpectedUwbLongRangeAgcTune2) {
    mask |= UwbLongRangeFailAgcTune2;
  }
  if (snapshot.drxTune0b != kExpectedUwbLongRangeDrxTune0b) {
    mask |= UwbLongRangeFailDrxTune0b;
  }
  if (snapshot.drxTune1a != kExpectedUwbLongRangeDrxTune1a) {
    mask |= UwbLongRangeFailDrxTune1a;
  }
  if (snapshot.drxTune1b != kExpectedUwbLongRangeDrxTune1b) {
    mask |= UwbLongRangeFailDrxTune1b;
  }
  if (snapshot.drxTune2 != kExpectedUwbLongRangeDrxTune2) {
    mask |= UwbLongRangeFailDrxTune2;
  }
  if (snapshot.drxSfdtoc != kExpectedUwbLongRangeDrxSfdtoc) {
    mask |= UwbLongRangeFailDrxSfdtoc;
  }
  if (snapshot.ldeCfg1 != kExpectedUwbLongRangeLdeCfg1) {
    mask |= UwbLongRangeFailLdeCfg1;
  }
  if (snapshot.ldeCfg2 != kExpectedUwbLongRangeLdeCfg2) {
    mask |= UwbLongRangeFailLdeCfg2;
  }
  if (snapshot.ldeRepc != kExpectedUwbLongRangeLdeRepc) {
    mask |= UwbLongRangeFailLdeRepc;
  }
  if ((snapshot.sysMask & kExpectedUwbLongRangeSysMaskMask) !=
      kExpectedUwbLongRangeSysMask) {
    mask |= UwbLongRangeFailSysMask;
  }
  return mask;
}

inline uint8_t countUwbLongRangeFailures(uint32_t mask) {
  uint8_t count = 0;
  while (mask != 0) {
    count += static_cast<uint8_t>(mask & 1UL);
    mask >>= 1;
  }
  return count;
}

inline const char* uwbLongRangeFailureName(uint32_t bit) {
  switch (bit) {
    case UwbLongRangeFailSysCfg:
      return "SYS_CFG";
    case UwbLongRangeFailChanCtrl:
      return "CHAN_CTRL";
    case UwbLongRangeFailTxFctrl:
      return "TX_FCTRL";
    case UwbLongRangeFailTxPower:
      return "TX_POWER";
    case UwbLongRangeFailRfTxctrl:
      return "RF_TXCTRL";
    case UwbLongRangeFailTcPgdelay:
      return "TC_PGDELAY";
    case UwbLongRangeFailFsPllcfg:
      return "FS_PLLCFG";
    case UwbLongRangeFailFsPlltune:
      return "FS_PLLTUNE";
    case UwbLongRangeFailAgcTune1:
      return "AGC_TUNE1";
    case UwbLongRangeFailAgcTune2:
      return "AGC_TUNE2";
    case UwbLongRangeFailDrxTune0b:
      return "DRX_TUNE0B";
    case UwbLongRangeFailDrxTune1a:
      return "DRX_TUNE1A";
    case UwbLongRangeFailDrxTune1b:
      return "DRX_TUNE1B";
    case UwbLongRangeFailDrxTune2:
      return "DRX_TUNE2";
    case UwbLongRangeFailDrxSfdtoc:
      return "DRX_SFDTOC";
    case UwbLongRangeFailLdeCfg1:
      return "LDE_CFG1";
    case UwbLongRangeFailLdeCfg2:
      return "LDE_CFG2";
    case UwbLongRangeFailLdeRepc:
      return "LDE_REPC";
    case UwbLongRangeFailSysMask:
      return "SYS_MASK";
    default:
      return "UNKNOWN";
  }
}

inline void appendText(char* output, size_t outputSize, const char* text) {
  if (output == nullptr || outputSize == 0 || text == nullptr) {
    return;
  }
  const size_t used = strlen(output);
  if (used >= outputSize - 1) {
    return;
  }
  snprintf(output + used, outputSize - used, "%s", text);
}

inline void formatUwbLongRangeFailureReason(uint32_t mask, char* output,
                                            size_t outputSize) {
  if (output == nullptr || outputSize == 0) {
    return;
  }
  output[0] = '\0';
  if (mask == 0) {
    appendText(output, outputSize, "NONE");
    return;
  }
  bool first = true;
  for (uint8_t i = 0; i < 32; ++i) {
    const uint32_t bit = 1UL << i;
    if ((mask & bit) == 0) {
      continue;
    }
    if (!first) {
      appendText(output, outputSize, ",");
    }
    appendText(output, outputSize, uwbLongRangeFailureName(bit));
    first = false;
  }
}
