#pragma once

#include <Arduino.h>

struct UwbDiagnosticState {
  bool spiReady = false;
  bool peerPresent = false;
  bool hasRange = false;
  bool rangeStale = false;
  float distanceM = 0.0f;
  float rawDistanceM = 0.0f;
  float rxPowerDbm = 0.0f;
  float quality = 0.0f;
  uint32_t lastRangeMs = 0;
  uint32_t rangeCount = 0;
  uint32_t rejectedCount = 0;
  uint32_t recoveryCount = 0;
};

struct Gy511DiagnosticState {
  bool initialized = false;
  bool readOk = false;
  int16_t accelX = 0;
  int16_t accelY = 0;
  int16_t accelZ = 0;
  int16_t magX = 0;
  int16_t magY = 0;
  int16_t magZ = 0;
  float headingDeg = 0.0f;
};

struct Max30102DiagnosticState {
  bool initialized = false;
  bool fingerPresent = false;
  long irValue = 0;
  float bpm = 0.0f;
  int averageBpm = 0;
};

struct DiagnosticsState {
  UwbDiagnosticState uwb;
  Gy511DiagnosticState gy511;
  Max30102DiagnosticState max30102;
};

