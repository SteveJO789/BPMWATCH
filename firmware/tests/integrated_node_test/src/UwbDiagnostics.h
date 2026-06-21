#pragma once

#include <Arduino.h>
#include <DW1000Ranging.h>

#include "DiagnosticsState.h"
#include "MedianRangeFilter.h"

class UwbDiagnostics {
 public:
  void begin(UwbDiagnosticState& state);
  void poll(uint32_t nowMs, UwbDiagnosticState& state);

 private:
  static UwbDiagnostics* instance_;
  static UwbDiagnosticState* state_;

  MedianRangeFilter rangeFilter_;
  uint32_t lastActivityMs_ = 0;
  bool hasEverConnected_ = false;
  char eui_[24]{};

  void restart(UwbDiagnosticState& state);
  void handleNewRange();
  void handleNewDevice(DW1000Device* device);
  void handleInactiveDevice(DW1000Device* device);
  void handleBlinkDevice(DW1000Device* device);

  static void onNewRange();
  static void onNewDevice(DW1000Device* device);
  static void onInactiveDevice(DW1000Device* device);
  static void onBlinkDevice(DW1000Device* device);
};

