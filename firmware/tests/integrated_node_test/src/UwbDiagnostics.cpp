#include "UwbDiagnostics.h"

#include <DW1000.h>
#include <SPI.h>
#include <cstring>

#include "DiagnosticsSync.h"
#include "NodeConfig.h"
#include "TimeUtils.h"
#include "UwbEventQueue.h"
#include "UwbLongRangeConfig.h"
#include "UwbRangingDeviceReset.h"

#ifndef UWB_ANTENNA_DELAY
#define UWB_ANTENNA_DELAY 16555
#endif

#ifndef UWB_DISCOVERY_RECOVERY_TIMEOUT_MS
#define UWB_DISCOVERY_RECOVERY_TIMEOUT_MS 5000
#endif

#ifndef UWB_CONNECTED_RECOVERY_TIMEOUT_MS
#define UWB_CONNECTED_RECOVERY_TIMEOUT_MS 15000
#endif

#ifndef UWB_RECEIVER_RECOVERY_COOLDOWN_MS
#define UWB_RECEIVER_RECOVERY_COOLDOWN_MS 3000
#endif

#ifndef BPMWATCH_TAG_UWB_RECOVERY
#define BPMWATCH_TAG_UWB_RECOVERY true
#endif

namespace {
constexpr int kUwbSck = 18;
constexpr int kUwbMiso = 19;
constexpr int kUwbMosi = 23;
constexpr int kUwbCs = 5;
constexpr int kUwbIrq = 34;
constexpr int kUwbReset = 4;
constexpr uint32_t kRangeStaleMs = 3000;
constexpr uint16_t kDrxSfdtocSub = 0x20;
constexpr uint8_t kLenDrxSfdtoc = 2;

constexpr uint8_t kLosNlosHintNone = 0;
constexpr uint8_t kLosNlosHintLos = 1;
constexpr uint8_t kLosNlosHintMaybe = 2;
constexpr uint8_t kLosNlosHintNlos = 3;

uint32_t packDw1000Register32(byte reg) {
  byte data[4]{};
  DW1000.readBytes(reg, NO_SUB, data, sizeof(data));
  uint32_t value = 0;
  for (uint8_t i = 0; i < sizeof(data); ++i) {
    value |= static_cast<uint32_t>(data[i]) << (8 * i);
  }
  return value;
}

uint32_t packDw1000RegisterSub32(byte reg, uint16_t sub) {
  byte data[4]{};
  DW1000.readBytes(reg, sub, data, sizeof(data));
  uint32_t value = 0;
  for (uint8_t i = 0; i < sizeof(data); ++i) {
    value |= static_cast<uint32_t>(data[i]) << (8 * i);
  }
  return value;
}

uint16_t packDw1000Register16(byte reg, uint16_t sub) {
  byte data[2]{};
  DW1000.readBytes(reg, sub, data, sizeof(data));
  return static_cast<uint16_t>(data[0]) |
         (static_cast<uint16_t>(data[1]) << 8);
}

uint8_t packDw1000Register8(byte reg, uint16_t sub) {
  byte data[1]{};
  DW1000.readBytes(reg, sub, data, sizeof(data));
  return data[0];
}

void writeDw1000RegisterLe(byte reg, uint16_t sub, uint32_t value,
                           uint8_t length) {
  byte data[4]{};
  for (uint8_t i = 0; i < length && i < sizeof(data); ++i) {
    data[i] = static_cast<byte>((value >> (8 * i)) & 0xFFU);
  }
  DW1000.writeBytes(reg, sub, data, length);
}

uint64_t packDw1000SystemStatus() {
  byte data[LEN_SYS_STATUS]{};
  DW1000.readBytes(SYS_STATUS, NO_SUB, data, sizeof(data));
  uint64_t value = 0;
  for (uint8_t i = 0; i < sizeof(data); ++i) {
    value |= static_cast<uint64_t>(data[i]) << (8 * i);
  }
  return value;
}

uint16_t packRxPacc() {
  byte rxFrameInfo[LEN_RX_FINFO]{};
  DW1000.readBytes(RX_FINFO, NO_SUB, rxFrameInfo, sizeof(rxFrameInfo));
  return static_cast<uint16_t>(((static_cast<uint16_t>(rxFrameInfo[2]) >> 4) &
                                0xFFU) |
                               (static_cast<uint16_t>(rxFrameInfo[3]) << 4));
}

UwbLongRangeRegisterSnapshot readLongRangeRegisterSnapshot() {
  UwbLongRangeRegisterSnapshot snapshot;
  const uint64_t sysStatus = packDw1000SystemStatus();
  snapshot.sysStatusLow = static_cast<uint32_t>(sysStatus & 0xFFFFFFFFULL);
  snapshot.sysStatusHigh =
      static_cast<uint8_t>((sysStatus >> 32) & 0xFFU);
  snapshot.sysMask = packDw1000Register32(SYS_MASK);
  snapshot.sysCfg = packDw1000Register32(SYS_CFG);
  snapshot.chanCtrl = packDw1000Register32(CHAN_CTRL);
  {
    byte txFctrl[LEN_TX_FCTRL]{};
    DW1000.readBytes(TX_FCTRL, NO_SUB, txFctrl, sizeof(txFctrl));
    snapshot.txFctrlLow =
        static_cast<uint32_t>(txFctrl[0]) |
        (static_cast<uint32_t>(txFctrl[1]) << 8) |
        (static_cast<uint32_t>(txFctrl[2]) << 16) |
        (static_cast<uint32_t>(txFctrl[3]) << 24);
    snapshot.txFctrlHigh = txFctrl[4];
  }
  snapshot.txPower = packDw1000Register32(TX_POWER);
  snapshot.rfTxctrl = packDw1000RegisterSub32(RF_CONF, RF_TXCTRL_SUB);
  snapshot.tcPgdelay = packDw1000Register8(TX_CAL, TC_PGDELAY_SUB);
  snapshot.fsPllcfg = packDw1000RegisterSub32(FS_CTRL, FS_PLLCFG_SUB);
  snapshot.fsPlltune = packDw1000Register8(FS_CTRL, FS_PLLTUNE_SUB);
  snapshot.agcTune1 = packDw1000Register16(AGC_TUNE, AGC_TUNE1_SUB);
  snapshot.agcTune2 = packDw1000RegisterSub32(AGC_TUNE, AGC_TUNE2_SUB);
  snapshot.drxTune0b = packDw1000Register16(DRX_TUNE, DRX_TUNE0b_SUB);
  snapshot.drxTune1a = packDw1000Register16(DRX_TUNE, DRX_TUNE1a_SUB);
  snapshot.drxTune1b = packDw1000Register16(DRX_TUNE, DRX_TUNE1b_SUB);
  snapshot.drxTune2 = packDw1000RegisterSub32(DRX_TUNE, DRX_TUNE2_SUB);
  snapshot.drxSfdtoc = packDw1000Register16(DRX_TUNE, kDrxSfdtocSub);
  snapshot.ldeCfg1 = packDw1000Register8(LDE_IF, LDE_CFG1_SUB);
  snapshot.ldeCfg2 = packDw1000Register16(LDE_IF, LDE_CFG2_SUB);
  snapshot.ldeRepc = packDw1000Register16(LDE_IF, LDE_REPC_SUB);
  return snapshot;
}
}  // namespace

UwbDiagnostics* UwbDiagnostics::instance_ = nullptr;
UwbDiagnosticState* UwbDiagnostics::state_ = nullptr;

void UwbDiagnostics::begin(UwbDiagnosticState& state) {
  instance_ = this;
  state_ = &state;
  std::strncpy(eui_, kNodeConfig.uwbEui, sizeof(eui_) - 1);
  SPI.begin(kUwbSck, kUwbMiso, kUwbMosi, kUwbCs);
  restart(state);
}

void UwbDiagnostics::poll(uint32_t nowMs, UwbDiagnosticState& state) {
  DW1000Ranging.loop();

  uint32_t observedRangeCount = 0;
  bool observedPeerPresent = false;
  bool observedHasRange = false;
  {
    DiagnosticsLock lock;
    ++state.uwbPollCount;
    state.rangeStale =
        state.hasRange && (safeAgeMs(nowMs, state.lastRangeMs) >= kRangeStaleMs);
    state.rangeAgeMs = state.hasRange ? safeAgeMs(nowMs, state.lastRangeMs) : 0;
    observedRangeCount = state.rangeCount;
    observedPeerPresent = state.peerPresent;
    observedHasRange = state.hasRange && !state.rangeStale;
  }

  if (nowMs - lastRegisterSnapshotMs_ >= 1000) {
    lastRegisterSnapshotMs_ = nowMs;
    dumpRegisterSnapshot(state);
    if (state.spiReady) {
      verifyLongRangeConfig(state, false);
    }
  }

  if (observedRangeCount != lastObservedRangeCount_) {
    lastObservedRangeCount_ = observedRangeCount;
    recoveryGate_.markActivity(nowMs);
    hasEverConnected_ = true;
  }
  if (observedPeerPresent && !lastObservedPeerPresent_) {
    recoveryGate_.markActivity(nowMs);
    hasEverConnected_ = true;
  }
  lastObservedPeerPresent_ = observedPeerPresent;

  {
    DiagnosticsLock lock;
    state.uwbActivityAgeMs = recoveryGate_.activityAgeMs(nowMs);
    state.uwbRecoveryAgeMs = recoveryGate_.recoveryAgeMs(nowMs);
    state.uwbRxFailureCount = DW1000Ranging.getReceiveFailureCount();
    state.uwbRxTimeoutCount = DW1000Ranging.getReceiveTimeoutCount();
    state.uwbReceiverResetCount = DW1000Ranging.getReceiverResetCount();
  }

  if (!kNodeConfig.isAnchor && !BPMWATCH_TAG_UWB_RECOVERY) {
    return;
  }

  if (observedHasRange) {
    return;
  }

  const uint32_t recoveryTimeoutMs =
      hasEverConnected_ ? UWB_CONNECTED_RECOVERY_TIMEOUT_MS
                        : UWB_DISCOVERY_RECOVERY_TIMEOUT_MS;

  // Stage 1: receiver-only reset (lightweight, re-arms receiver)
  uint32_t rxFailCount = 0;
  uint32_t rxTimeoutCount = 0;
  {
    DiagnosticsLock lock;
    rxFailCount = state.uwbRxFailureCount;
    rxTimeoutCount = state.uwbRxTimeoutCount;
  }

  if (recoveryGate_.shouldRecover(nowMs, UWB_RECEIVER_RECOVERY_COOLDOWN_MS)) {
    if (rxFailCount > 0 || rxTimeoutCount > 0) {
      receiverRecover(state);
      return;
    }
  }

  // Stage 2: full restart only after extended inactivity
  if (recoveryGate_.shouldRecover(nowMs, recoveryTimeoutMs)) {
    uint32_t recoveryCount = 0;
    {
      DiagnosticsLock lock;
      recoveryCount = ++state.recoveryCount;
    }
    pushUwbEvent(
        {UwbEventType::Recovery, recoveryCount, recoveryTimeoutMs, {}});
    recoveryGate_.markRecovery(nowMs);
    restart(state);
  }
}

void UwbDiagnostics::receiverRecover(UwbDiagnosticState& state) {
  {
    DiagnosticsLock lock;
    ++state.uwbReceiverResetCount;
    state.peerPresent = false;
    state.hasRange = false;
    state.rangeStale = false;
    state.rangeAgeMs = 0;
  }

  recoveryGate_.markActivity(millis());
  DW1000.idle();
  DW1000.resetReceiver();
  DW1000.commitConfiguration();
  dumpRegisterSnapshot(state);
  verifyLongRangeConfig(state, true);
  pushUwbEvent(
      {UwbEventType::Recovery, 0, UWB_RECEIVER_RECOVERY_COOLDOWN_MS, {}});
}

void UwbDiagnostics::configureDw1000LongRangeMode() {
  DW1000.newConfiguration();
  DW1000.setDefaults();
  DW1000.setAntennaDelay(UWB_ANTENNA_DELAY);
  DW1000.enableMode(DW1000.MODE_LONGDATA_RANGE_ACCURACY);
  DW1000.setChannel(DW1000.CHANNEL_5);
  DW1000.setPreambleCode(DW1000.PREAMBLE_CODE_64MHZ_10);
  DW1000.useSmartPower(false);
  DW1000.interruptOnReceiveTimeout(true);
  DW1000.commitConfiguration();
  writeDw1000RegisterLe(DRX_TUNE, kDrxSfdtocSub,
                        kExpectedUwbLongRangeDrxSfdtoc, kLenDrxSfdtoc);
  DW1000.newReceive();
  DW1000.receivePermanently(true);
  DW1000.startReceive();
}

void UwbDiagnostics::copyLongRangeSnapshotToState(
    const UwbLongRangeRegisterSnapshot& snapshot,
    UwbDiagnosticState& state) {
  state.uwbSysStatusLow = snapshot.sysStatusLow;
  state.uwbSysStatusHigh = snapshot.sysStatusHigh;
  state.uwbSysMask = snapshot.sysMask;
  state.uwbSysCfg = snapshot.sysCfg;
  state.regChanCtrl = snapshot.chanCtrl;
  state.regTxFctrlLow = snapshot.txFctrlLow;
  state.regTxFctrlHigh = snapshot.txFctrlHigh;
  state.regDrxTune0b = snapshot.drxTune0b;
  state.regDrxTune1a = snapshot.drxTune1a;
  state.regDrxTune1b = snapshot.drxTune1b;
  state.regDrxTune2 = snapshot.drxTune2;
  state.regDrxSfdtoc = snapshot.drxSfdtoc;
  state.regAgcTune1 = snapshot.agcTune1;
  state.regAgcTune2 = snapshot.agcTune2;
  state.regLdeCfg1 = snapshot.ldeCfg1;
  state.regLdeCfg2 = snapshot.ldeCfg2;
  state.regLdeRepc = snapshot.ldeRepc;
  state.regTxPower = snapshot.txPower;
  state.regRfTxctrl = snapshot.rfTxctrl;
  state.regTcPgdelay = snapshot.tcPgdelay;
  state.regFsPllcfg = snapshot.fsPllcfg;
  state.regFsPlltune = snapshot.fsPlltune;
}

void UwbDiagnostics::dumpRegisterSnapshot(UwbDiagnosticState& state) {
  const UwbLongRangeRegisterSnapshot snapshot = readLongRangeRegisterSnapshot();
  const uint32_t sysCtrl = packDw1000Register32(SYS_CTRL);
  const uint8_t deviceMode = DW1000._deviceMode;
  DiagnosticsLock lock;
  copyLongRangeSnapshotToState(snapshot, state);
  state.uwbSysCtrl = sysCtrl;
  state.uwbDeviceMode = deviceMode;
}

void UwbDiagnostics::verifyLongRangeConfig(UwbDiagnosticState& state,
                                           bool forceFullDump) {
  const UwbLongRangeRegisterSnapshot snapshot = readLongRangeRegisterSnapshot();
  const uint32_t failureMask = uwbLongRangeFailureMask(snapshot);
  const uint8_t failures = countUwbLongRangeFailures(failureMask);
  char reason[sizeof(state.regConfigFailureReason)]{};
  formatUwbLongRangeFailureReason(failureMask, reason, sizeof(reason));

  {
    DiagnosticsLock lock;
    copyLongRangeSnapshotToState(snapshot, state);
    state.regConfigFailureMask = failureMask;
    state.regConfigFailures = failures;
    state.longRangeConfigOk = (failures == 0);
    std::strncpy(state.regConfigFailureReason, reason,
                 sizeof(state.regConfigFailureReason) - 1);
    state.regConfigFailureReason[sizeof(state.regConfigFailureReason) - 1] =
        '\0';
  }

  const bool failureReasonChanged =
      failureMask != lastLoggedLongRangeFailureMask_;
  if (forceFullDump || failureReasonChanged) {
    Serial.printf("LR_OK=%d LR_FAIL=%u LR_FAIL_REASON=%s\n",
                  failures == 0 ? 1 : 0, static_cast<unsigned>(failures),
                  reason);
    logLongRangeRegisterSnapshot(snapshot, failureMask);
    lastLoggedLongRangeFailureMask_ = failureMask;
  }
}

void UwbDiagnostics::logLongRangeRegisterSnapshot(
    const UwbLongRangeRegisterSnapshot& snapshot, uint32_t failureMask) {
  Serial.printf(
      "LR_REG=SYS_CFG expected=0x%08lX mask=0x%08lX actual=0x%08lX %s\n",
      static_cast<unsigned long>(kExpectedUwbLongRangeSysCfg),
      static_cast<unsigned long>(kExpectedUwbLongRangeSysCfgMask),
      static_cast<unsigned long>(snapshot.sysCfg),
      (failureMask & UwbLongRangeFailSysCfg) ? "FAIL" : "OK");
  Serial.printf(
      "LR_REG=CHAN_CTRL expected=0x%08lX actual=0x%08lX %s\n",
      static_cast<unsigned long>(kExpectedUwbLongRangeChanCtrl),
      static_cast<unsigned long>(snapshot.chanCtrl),
      (failureMask & UwbLongRangeFailChanCtrl) ? "FAIL" : "OK");
  Serial.printf(
      "LR_REG=TX_FCTRL expected=0x%08lX mask=0x%08lX actual=0x%02X%08lX %s\n",
      static_cast<unsigned long>(kExpectedUwbLongRangeTxFctrl),
      static_cast<unsigned long>(kExpectedUwbLongRangeTxFctrlMask),
      static_cast<unsigned>(snapshot.txFctrlHigh),
      static_cast<unsigned long>(snapshot.txFctrlLow),
      (failureMask & UwbLongRangeFailTxFctrl) ? "FAIL" : "OK");
  Serial.printf("LR_REG=TX_POWER expected=0x%08lX actual=0x%08lX %s\n",
                static_cast<unsigned long>(kExpectedUwbLongRangeTxPower),
                static_cast<unsigned long>(snapshot.txPower),
                (failureMask & UwbLongRangeFailTxPower) ? "FAIL" : "OK");
  Serial.printf(
      "LR_REG=RF_TXCTRL expected=0x%08lX mask=0x%08lX actual=0x%08lX %s\n",
      static_cast<unsigned long>(kExpectedUwbLongRangeRfTxctrl),
      static_cast<unsigned long>(kExpectedUwbLongRangeRfTxctrlMask),
      static_cast<unsigned long>(snapshot.rfTxctrl),
      (failureMask & UwbLongRangeFailRfTxctrl) ? "FAIL" : "OK");
  Serial.printf("LR_REG=TC_PGDELAY expected=0x%02X actual=0x%02X %s\n",
                kExpectedUwbLongRangeTcPgdelay, snapshot.tcPgdelay,
                (failureMask & UwbLongRangeFailTcPgdelay) ? "FAIL" : "OK");
  Serial.printf("LR_REG=FS_PLLCFG expected=0x%08lX actual=0x%08lX %s\n",
                static_cast<unsigned long>(kExpectedUwbLongRangeFsPllcfg),
                static_cast<unsigned long>(snapshot.fsPllcfg),
                (failureMask & UwbLongRangeFailFsPllcfg) ? "FAIL" : "OK");
  Serial.printf("LR_REG=FS_PLLTUNE expected=0x%02X actual=0x%02X %s\n",
                kExpectedUwbLongRangeFsPlltune, snapshot.fsPlltune,
                (failureMask & UwbLongRangeFailFsPlltune) ? "FAIL" : "OK");
  Serial.printf("LR_REG=AGC_TUNE1 expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeAgcTune1, snapshot.agcTune1,
                (failureMask & UwbLongRangeFailAgcTune1) ? "FAIL" : "OK");
  Serial.printf("LR_REG=AGC_TUNE2 expected=0x%08lX actual=0x%08lX %s\n",
                static_cast<unsigned long>(kExpectedUwbLongRangeAgcTune2),
                static_cast<unsigned long>(snapshot.agcTune2),
                (failureMask & UwbLongRangeFailAgcTune2) ? "FAIL" : "OK");
  Serial.printf("LR_REG=DRX_TUNE0B expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeDrxTune0b, snapshot.drxTune0b,
                (failureMask & UwbLongRangeFailDrxTune0b) ? "FAIL" : "OK");
  Serial.printf("LR_REG=DRX_TUNE1A expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeDrxTune1a, snapshot.drxTune1a,
                (failureMask & UwbLongRangeFailDrxTune1a) ? "FAIL" : "OK");
  Serial.printf("LR_REG=DRX_TUNE1B expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeDrxTune1b, snapshot.drxTune1b,
                (failureMask & UwbLongRangeFailDrxTune1b) ? "FAIL" : "OK");
  Serial.printf("LR_REG=DRX_TUNE2 expected=0x%08lX actual=0x%08lX %s\n",
                static_cast<unsigned long>(kExpectedUwbLongRangeDrxTune2),
                static_cast<unsigned long>(snapshot.drxTune2),
                (failureMask & UwbLongRangeFailDrxTune2) ? "FAIL" : "OK");
  Serial.printf("LR_REG=DRX_SFDTOC expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeDrxSfdtoc, snapshot.drxSfdtoc,
                (failureMask & UwbLongRangeFailDrxSfdtoc) ? "FAIL" : "OK");
  Serial.printf("LR_REG=LDE_CFG1 expected=0x%02X actual=0x%02X %s\n",
                kExpectedUwbLongRangeLdeCfg1, snapshot.ldeCfg1,
                (failureMask & UwbLongRangeFailLdeCfg1) ? "FAIL" : "OK");
  Serial.printf("LR_REG=LDE_CFG2 expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeLdeCfg2, snapshot.ldeCfg2,
                (failureMask & UwbLongRangeFailLdeCfg2) ? "FAIL" : "OK");
  Serial.printf("LR_REG=LDE_REPC expected=0x%04X actual=0x%04X %s\n",
                kExpectedUwbLongRangeLdeRepc, snapshot.ldeRepc,
                (failureMask & UwbLongRangeFailLdeRepc) ? "FAIL" : "OK");
  Serial.printf(
      "LR_REG=SYS_MASK expected=0x%08lX mask=0x%08lX actual=0x%08lX %s\n",
      static_cast<unsigned long>(kExpectedUwbLongRangeSysMask),
      static_cast<unsigned long>(kExpectedUwbLongRangeSysMaskMask),
      static_cast<unsigned long>(snapshot.sysMask),
      (failureMask & UwbLongRangeFailSysMask) ? "FAIL" : "OK");
  Serial.printf("LR_REG=SYS_STATUS actual=0x%02X%08lX\n",
                static_cast<unsigned>(snapshot.sysStatusHigh),
                static_cast<unsigned long>(snapshot.sysStatusLow));
}

void UwbDiagnostics::captureRxQuality(DW1000Device* device,
                                      UwbDiagnosticState& state) {
  if (device == nullptr) {
    return;
  }

  float rxFpPower = device->getFPPower();
  float rxPower = device->getRXPower();
  float delta = rxPower - rxFpPower;
  const uint16_t rxPacc = packRxPacc();
  const uint16_t cirPower = packDw1000Register16(RX_FQUAL, CIR_PWR_SUB);
  const uint16_t fpAmpl1 = packDw1000Register16(RX_TIME, FP_AMPL1_SUB);
  const uint16_t fpAmpl2 = packDw1000Register16(RX_FQUAL, FP_AMPL2_SUB);
  const uint16_t fpAmpl3 = packDw1000Register16(RX_FQUAL, FP_AMPL3_SUB);
  const uint16_t stdNoise = packDw1000Register16(RX_FQUAL, STD_NOISE_SUB);

  DiagnosticsLock lock;
  state.rxFpPowerDbm = rxFpPower;
  state.rxLosNlosDelta = delta;
  state.rxPacc = rxPacc;
  state.rxCirPower = cirPower;
  state.rxFpAmpl1 = fpAmpl1;
  state.rxFpAmpl2 = fpAmpl2;
  state.rxFpAmpl3 = fpAmpl3;
  state.rxStdNoise = stdNoise;

  if (delta < 6.0f) {
    state.rxLosNlosHint = kLosNlosHintLos;
  } else if (delta > 10.0f) {
    state.rxLosNlosHint = kLosNlosHintNlos;
  } else {
    state.rxLosNlosHint = kLosNlosHintMaybe;
  }
}

void UwbDiagnostics::restart(UwbDiagnosticState& state) {
  clearUwbRangingDevices(DW1000Ranging);
  {
    DiagnosticsLock lock;
    state.peerPresent = false;
    state.hasRange = false;
    state.rangeStale = false;
    state.rangeAgeMs = 0;
    ++state.uwbRestartCount;
    state.longRangeConfigOk = false;
    state.regConfigFailures = 0;
  }
  lastObservedPeerPresent_ = false;
  lastRegisterSnapshotMs_ = 0;
  lastLoggedLongRangeFailureMask_ = UINT32_MAX;
  rangeFilter_.reset();

  DW1000Ranging.initCommunication(kUwbReset, kUwbCs, kUwbIrq);
  char deviceId[64] = "not checked";
  DW1000.getPrintableDeviceIdentifier(deviceId);
  {
    DiagnosticsLock lock;
    state.spiReady = std::strncmp(deviceId, "DECA", 4) == 0;
  }
  UwbEvent event{UwbEventType::DeviceId, 0, 0, {}};
  std::strncpy(event.text, deviceId, sizeof(event.text) - 1);
  pushUwbEvent(event);

  DW1000.setAntennaDelay(UWB_ANTENNA_DELAY);
  DW1000Ranging.attachNewRange(onNewRange);
  DW1000Ranging.attachNewDevice(onNewDevice);
  if (kNodeConfig.isAnchor) {
    DW1000Ranging.attachBlinkDevice(onBlinkDevice);
  }
  DW1000Ranging.attachInactiveDevice(onInactiveDevice);
  DW1000Ranging.useRangeFilter(false);

  if (kNodeConfig.isAnchor) {
    DW1000Ranging.startAsAnchor(eui_, DW1000.MODE_LONGDATA_RANGE_ACCURACY,
                                false);
  } else {
    DW1000Ranging.startAsTag(eui_, DW1000.MODE_LONGDATA_RANGE_ACCURACY,
                             false);
  }

  configureDw1000LongRangeMode();

  // Dump registers immediately after init for sanity check
  dumpRegisterSnapshot(state);
  verifyLongRangeConfig(state, true);

  recoveryGate_.markActivity(millis());
}

void UwbDiagnostics::handleNewRange() {
  DW1000Device* device = DW1000Ranging.getDistantDevice();
  if (device == nullptr || state_ == nullptr) {
    return;
  }

  const uint32_t nowMs = millis();
  const float rawDistanceM = device->getRange();
  recoveryGate_.markActivity(nowMs);
  hasEverConnected_ = true;

  if (!rangeFilter_.add(rawDistanceM)) {
    DiagnosticsLock lock;
    ++state_->uwbRangeEventCount;
    state_->rawDistanceM = rawDistanceM;
    state_->peerPresent = true;
    state_->rejectedCount = rangeFilter_.rejectedCount();
    return;
  }

  const float distanceM = rangeFilter_.value();
  const float rxPowerDbm = device->getRXPower();
  const float quality = device->getQuality();
  const uint32_t rejectedCount = rangeFilter_.rejectedCount();

  captureRxQuality(device, *state_);

  DiagnosticsLock lock;
  ++state_->uwbRangeEventCount;
  state_->rawDistanceM = rawDistanceM;
  state_->peerPresent = true;
  state_->hasRange = true;
  state_->rangeStale = false;
  state_->distanceM = distanceM;
  state_->rxPowerDbm = rxPowerDbm;
  state_->quality = quality;
  state_->lastRangeMs = nowMs;
  ++state_->rangeCount;
  state_->rejectedCount = rejectedCount;
}

void UwbDiagnostics::handleNewDevice(DW1000Device* device) {
  if (state_ == nullptr || device == nullptr) {
    return;
  }
  {
    DiagnosticsLock lock;
    ++state_->uwbPeerEventCount;
    state_->peerPresent = true;
  }
  hasEverConnected_ = true;
  recoveryGate_.markActivity(millis());
  pushUwbEvent({UwbEventType::PeerAdded, device->getShortAddress(), 0, {}});
}

void UwbDiagnostics::handleInactiveDevice(DW1000Device* device) {
  if (state_ != nullptr) {
    DiagnosticsLock lock;
    ++state_->uwbInactiveEventCount;
    state_->peerPresent = false;
  }
  if (device != nullptr) {
    pushUwbEvent(
        {UwbEventType::PeerInactive, device->getShortAddress(), 0, {}});
  }
}

void UwbDiagnostics::handleBlinkDevice(DW1000Device* device) {
  handleNewDevice(device);
}

void UwbDiagnostics::onNewRange() {
  if (instance_ != nullptr) {
    instance_->handleNewRange();
  }
}

void UwbDiagnostics::onNewDevice(DW1000Device* device) {
  if (instance_ != nullptr) {
    instance_->handleNewDevice(device);
  }
}

void UwbDiagnostics::onInactiveDevice(DW1000Device* device) {
  if (instance_ != nullptr) {
    instance_->handleInactiveDevice(device);
  }
}

void UwbDiagnostics::onBlinkDevice(DW1000Device* device) {
  if (instance_ != nullptr) {
    instance_->handleBlinkDevice(device);
  }
}
