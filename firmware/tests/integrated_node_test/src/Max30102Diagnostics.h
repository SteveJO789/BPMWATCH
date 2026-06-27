#pragma once

#include <stdint.h>

#include "FeatureFlags.h"
#include "Max30102SignalReason.h"

enum Max30102BpmZeroReason : uint8_t {
  Max30102BpmReasonOk = 0,
  Max30102BpmReasonOff,
  Max30102BpmReasonNoFinger,
  Max30102BpmReasonLowSps,
  Max30102BpmReasonLowIrAc,
  Max30102BpmReasonLowIrAcRatio,
  Max30102BpmReasonLowSignal,
  Max30102BpmReasonNoBeat,
  Max30102BpmReasonRefractory,
  Max30102BpmReasonRejectedBeat,
  Max30102BpmReasonUnstableBeat,
  Max30102BpmReasonWaitStable,
  Max30102BpmReasonWait
};

inline Max30102BpmZeroReason max30102BpmZeroReason(
    bool initialized, bool fingerPresent, bool signalUsable, bool bpmValid,
    uint8_t stableBeatCount, uint32_t sps, long irAc,
    uint32_t irAcRatioPpm, uint32_t beatCount, uint32_t rejectedBeatCount,
    uint32_t refractoryBeatRejectCount, uint32_t intervalBeatRejectCount,
    uint32_t unstableBeatRejectCount) {
  if (!initialized) {
    return Max30102BpmReasonOff;
  }
  if (!fingerPresent) {
    return Max30102BpmReasonNoFinger;
  }
  if (sps > 0 && sps < BPMWATCH_MAX30102_MIN_SPS) {
    return Max30102BpmReasonLowSps;
  }
  if (!signalUsable && irAc < BPMWATCH_MAX30102_MIN_IR_AC) {
    return Max30102BpmReasonLowIrAc;
  }
  if (!signalUsable &&
      irAcRatioPpm < BPMWATCH_MAX30102_MIN_IR_AC_RATIO_PPM) {
    return Max30102BpmReasonLowIrAcRatio;
  }
  if (!signalUsable) {
    return Max30102BpmReasonLowSignal;
  }
  if (bpmValid && signalUsable) {
    return Max30102BpmReasonOk;
  }
  if (beatCount == 0) {
    return Max30102BpmReasonNoBeat;
  }
  if (stableBeatCount < BPMWATCH_MAX30102_REQUIRED_STABLE_BEATS) {
    return Max30102BpmReasonWaitStable;
  }
  if (unstableBeatRejectCount > 0) {
    return Max30102BpmReasonUnstableBeat;
  }
  if (refractoryBeatRejectCount > 0) {
    return Max30102BpmReasonRefractory;
  }
  if (intervalBeatRejectCount > 0 || rejectedBeatCount > 0) {
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
    case Max30102BpmReasonLowIrAcRatio:
      return "LOW_IRAC_RATIO";
    case Max30102BpmReasonLowSignal:
      return "LOW_SIGNAL";
    case Max30102BpmReasonNoBeat:
      return "NO_BEAT";
    case Max30102BpmReasonRefractory:
      return "REFRACTORY";
    case Max30102BpmReasonRejectedBeat:
      return "BEAT_REJECT";
    case Max30102BpmReasonUnstableBeat:
      return "UNSTABLE_BEAT";
    case Max30102BpmReasonWaitStable:
      return "WAIT_STABLE";
    case Max30102BpmReasonWait:
    default:
      return "WAIT";
  }
}
