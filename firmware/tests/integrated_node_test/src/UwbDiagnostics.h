#pragma once

#include <Arduino.h>
#include <DW1000Ranging.h>

#include "DiagnosticsState.h"
#include "MedianRangeFilter.h"
#include "UwbLongRangeConfig.h"
#include "UwbRecoveryGate.h"

class DW1000Device;

class UwbDiagnostics {
 public:
  void begin(UwbDiagnosticState& state);
  void poll(uint32_t nowMs, UwbDiagnosticState& state);

 private:
  static UwbDiagnostics* instance_;
  static UwbDiagnosticState* state_;

  MedianRangeFilter rangeFilter_;
  UwbRecoveryGate recoveryGate_;
  uint32_t lastObservedRangeCount_ = 0;
  uint32_t lastRegisterSnapshotMs_ = 0;
  bool lastObservedPeerPresent_ = false;
  bool hasEverConnected_ = false;
  char eui_[24]{};

  void restart(UwbDiagnosticState& state);
  void receiverRecover(UwbDiagnosticState& state);
  void configureDw1000LongRangeMode();
  void dumpRegisterSnapshot(UwbDiagnosticState& state);
  void verifyLongRangeConfig(UwbDiagnosticState& state);
  void copyLongRangeSnapshotToState(
      const UwbLongRangeRegisterSnapshot& snapshot,
      UwbDiagnosticState& state);
  void logLongRangeRegisterSnapshot(
      const UwbLongRangeRegisterSnapshot& snapshot,
      uint32_t failureMask);
  void captureRxQuality(DW1000Device* device, UwbDiagnosticState& state);
  void handleNewRange();
  void handleNewDevice(DW1000Device* device);
  void handleInactiveDevice(DW1000Device* device);
  void handleBlinkDevice(DW1000Device* device);

  static void onNewRange();
  static void onNewDevice(DW1000Device* device);
  static void onInactiveDevice(DW1000Device* device);
  static void onBlinkDevice(DW1000Device* device);
};
