#pragma once

#include <Arduino.h>

#include "CompassSensor.h"
#include "RadarMap.h"
#include "SosButton.h"

struct UwbDiagnosticState {
  bool spiReady = false;
  bool peerPresent = false;
  bool hasRange = false;
  bool rangeStale = false;
  bool espNowReady = false;
  float distanceM = 0.0f;
  float rawDistanceM = 0.0f;
  float rxPowerDbm = 0.0f;
  float quality = 0.0f;
  uint32_t lastRangeMs = 0;
  uint32_t rangeCount = 0;
  uint32_t rejectedCount = 0;
  uint32_t recoveryCount = 0;
  uint32_t rangeAgeMs = 0;
  uint32_t uwbActivityAgeMs = 0;
  uint32_t uwbRecoveryAgeMs = 0;
  uint32_t uwbInterruptCount = 0;
  uint32_t uwbPollCount = 0;
  uint32_t uwbRangeEventCount = 0;
  uint32_t uwbPeerEventCount = 0;
  uint32_t uwbInactiveEventCount = 0;
  uint32_t uwbRestartCount = 0;
  uint32_t uwbRxFailureCount = 0;
  uint32_t uwbRxTimeoutCount = 0;
  uint32_t uwbReceiverResetCount = 0;
  uint32_t uwbSysStatusLow = 0;
  uint8_t uwbSysStatusHigh = 0;
  uint32_t uwbSysMask = 0;
  uint32_t uwbSysCtrl = 0;
  uint32_t uwbSysCfg = 0;
  uint8_t uwbDeviceMode = 0;
  // DW1000 register sanity dump (refreshed periodically)
  uint32_t regChanCtrl = 0;
  uint32_t regTxFctrlLow = 0;
  uint8_t regTxFctrlHigh = 0;
  uint16_t regDrxTune0b = 0;
  uint16_t regDrxTune1a = 0;
  uint16_t regDrxTune1b = 0;
  uint32_t regDrxTune2 = 0;
  uint16_t regDrxSfdtoc = 0;
  uint16_t regAgcTune1 = 0;
  uint32_t regAgcTune2 = 0;
  uint8_t regLdeCfg1 = 0;
  uint16_t regLdeCfg2 = 0;
  uint16_t regLdeRepc = 0;
  uint32_t regTxPower = 0;
  uint32_t regRfTxctrl = 0;
  uint8_t regTcPgdelay = 0;
  uint32_t regFsPllcfg = 0;
  uint8_t regFsPlltune = 0;
  // Config verification (set once after init)
  bool longRangeConfigOk = false;
  uint8_t regConfigFailures = 0;
  uint32_t regConfigFailureMask = 0;
  char regConfigFailureReason[96] = "NONE";
  // RX quality diagnostics (per-range)
  float rxFpPowerDbm = 0.0f;
  float rxLosNlosDelta = 0.0f;
  uint8_t rxLosNlosHint = 0;
  uint16_t rxPacc = 0;
  uint16_t rxCirPower = 0;
  uint16_t rxFpAmpl1 = 0;
  uint16_t rxFpAmpl2 = 0;
  uint16_t rxFpAmpl3 = 0;
  uint16_t rxStdNoise = 0;
  uint8_t uwbIrqPinLevel = 0;
  uint32_t espNowRxCount = 0;
  uint32_t espNowTxCount = 0;
  uint32_t espNowTxFailCount = 0;
  uint32_t lastEspNowRxMs = 0;
  uint32_t uwbTaskStackHighWater = 0;
};

struct Max30102DiagnosticState {
  bool initialized = false;
  bool fingerPresent = false;
  long irValue = 0;
  float bpm = 0.0f;
  int averageBpm = 0;
  // Diagnostics added for sampling rate & beat detection visibility
  uint32_t maxSampleCount = 0;
  uint32_t maxSampleResetMs = 0;
  uint32_t maxSps = 0;
  uint32_t maxBeatDetectCount = 0;
  uint32_t lastBeatAgeMs = 0;
  long maxIrMin1s = 0;
  long maxIrMax1s = 0;
  long maxIrAc1s = 0;
  uint32_t rejectedBeatCount = 0;
  uint32_t lastBeatIntervalMs = 0;
  uint32_t maxSampleDurationUs = 0;
  uint32_t maxSampleDurationMaxUs = 0;
  uint32_t maxLockFailCount = 0;
  uint32_t maxTaskStackHighWater = 0;
  bool maxTaskCreated = false;
};

struct PeerTelemetryState {
  bool headingValid = false;
  float headingDeg = 0.0f;
  bool bpmValid = false;
  bool bpmLost = false;
  uint16_t bpm = 0;
  uint32_t lastBpmRxMs = 0;
  uint8_t nodeId = 0;
  uint32_t lastRxMs = 0;
  RemoteSosState remoteSos;
};

struct DiagnosticsState {
  UwbDiagnosticState uwb;
  CompassDiagnosticState compass;
  Max30102DiagnosticState max30102;
  SosButtonState sos;
  PeerTelemetryState peer;
  RadarState radar;
};
