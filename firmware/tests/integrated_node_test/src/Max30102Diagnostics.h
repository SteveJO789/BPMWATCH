#pragma once

#include <stdint.h>

enum Max30102BpmZeroReason : uint8_t {
  Max30102BpmReasonOk = 0,
  Max30102BpmReasonOff,
  Max30102BpmReasonNoFinger,
  Max30102BpmReasonLowSps,
  Max30102BpmReasonLowIrAc,
  Max30102BpmReasonNoBeat,
  Max30102BpmReasonRejectedBeat,
  Max30102BpmReasonWait
};

inline Max30102BpmZeroReason max30102BpmZeroReason(
    bool initialized, bool fingerPresent, int averageBpm, uint32_t sps,
    long irAc, uint32_t beatCount, uint32_t rejectedBeatCount) {
  if (!initialized) {
    return Max30102BpmReasonOff;
  }
  if (!fingerPresent) {
    return Max30102BpmReasonNoFinger;
  }
  if (averageBpm > 0) {
    return Max30102BpmReasonOk;
  }
  if (sps > 0 && sps < 70) {
    return Max30102BpmReasonLowSps;
  }
  if (sps >= 70 && irAc < 100) {
    return Max30102BpmReasonLowIrAc;
  }
  if (beatCount == 0) {
    return Max30102BpmReasonNoBeat;
  }
  if (rejectedBeatCount > 0) {
    return Max30102BpmReasonRejectedBeat;
  }
  return Max30102BpmReasonWait;
}

inline const char* max30102BpmZeroReasonLabel(
    Max30102BpmZeroReason reason) {
  switch (reason) {
    case Max30102BpmReasonOk:
      return "OK";
    case Max30102BpmReasonOff:
      return "OFF";
    case Max30102BpmReasonNoFinger:
      return "NO_FINGER";
    case Max30102BpmReasonLowSps:
      return "LOW_SPS";
    case Max30102BpmReasonLowIrAc:
      return "LOW_IRAC";
    case Max30102BpmReasonNoBeat:
      return "NO_BEAT";
    case Max30102BpmReasonRejectedBeat:
      return "BEAT_REJECT";
    case Max30102BpmReasonWait:
    default:
      return "WAIT";
  }
}
