#pragma once

#include <stdint.h>

enum Max30102SignalOkReason : uint8_t {
  Max30102SignalReasonOff = 0,
  Max30102SignalReasonNoFinger,
  Max30102SignalReasonLowSps,
  Max30102SignalReasonLowIrAc,
  Max30102SignalReasonLowIrAcRatio,
  Max30102SignalReasonNormal,
  Max30102SignalReasonWristEnter,
  Max30102SignalReasonWristHold,
  Max30102SignalReasonHoldLost,
  Max30102SignalReasonLost
};

inline const char* max30102SignalOkReasonLabel(uint8_t reason) {
  switch (reason) {
    case Max30102SignalReasonOff:
      return "OFF";
    case Max30102SignalReasonNoFinger:
      return "NO_FINGER";
    case Max30102SignalReasonLowSps:
      return "LOW_SPS";
    case Max30102SignalReasonLowIrAc:
      return "LOW_IRAC";
    case Max30102SignalReasonLowIrAcRatio:
      return "LOW_IRAC_RATIO";
    case Max30102SignalReasonNormal:
      return "NORMAL";
    case Max30102SignalReasonWristEnter:
      return "WRIST_ENTER";
    case Max30102SignalReasonWristHold:
      return "WRIST_HOLD";
    case Max30102SignalReasonHoldLost:
      return "HOLD_LOST";
    case Max30102SignalReasonLost:
      return "LOST";
    default:
      return "UNKNOWN";
  }
}
