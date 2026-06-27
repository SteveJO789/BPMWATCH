#include "FeatureFlags.h"

#if BPMWATCH_ENABLE_MAX30102
#include "Max30102Sensor.h"

#include <heartRate.h>

#include "Max30102BeatDecision.h"
#include "TimeUtils.h"

#ifndef BPMWATCH_MAX30102_IR_AMPLITUDE
#define BPMWATCH_MAX30102_IR_AMPLITUDE 0x1F
#endif

bool Max30102Sensor::begin(TwoWire& wire) {
  if (!sensor_.begin(wire, I2C_SPEED_STANDARD)) {
    return false;
  }
  sensor_.setup(BPMWATCH_MAX30102_IR_AMPLITUDE, 1, 2, 100, 411, 4096);
  sensor_.setPulseAmplitudeRed(0x0A);
  sensor_.setPulseAmplitudeIR(BPMWATCH_MAX30102_IR_AMPLITUDE);
  sensor_.setPulseAmplitudeGreen(0);
  resetInternalBeatState();
  return true;
}

void Max30102Sensor::resetInternalBeatState() {
  rateSpot_ = 0;
  validRateCount_ = 0;
  intervalSpot_ = 0;
  validIntervalCount_ = 0;
  lastRawBeatMs_ = 0;
  lastAcceptedBeatMs_ = 0;
  lastAcceptedIntervalMs_ = 0;
  signalLostSinceMs_ = 0;
  for (uint8_t& rate : rates_) {
    rate = 0;
  }
  for (uint16_t& interval : intervals_) {
    interval = 0;
  }
}

void Max30102Sensor::clearAcceptedIntervalState(
    Max30102DiagnosticState& state) {
  rateSpot_ = 0;
  validRateCount_ = 0;
  intervalSpot_ = 0;
  validIntervalCount_ = 0;
  lastAcceptedIntervalMs_ = 0;
  for (uint8_t& rate : rates_) {
    rate = 0;
  }
  for (uint16_t& interval : intervals_) {
    interval = 0;
  }
  state.bpm = 0.0f;
  state.averageBpm = 0;
  state.bpmValid = false;
  state.stableBeatCount = 0;
  state.acceptedBeatCount = 0;
  state.acceptedBeatIntervalMs = 0;
  state.medianBeatIntervalMs = 0;
}

void Max30102Sensor::clearBeatTimingState(Max30102DiagnosticState& state) {
  clearAcceptedIntervalState(state);
  lastRawBeatMs_ = 0;
  lastAcceptedBeatMs_ = 0;
  state.acceptedBeatSeeded = false;
  state.rawBeatIntervalMs = 0;
  state.candidateBeatIntervalMs = 0;
  state.lastBeatIntervalMs = 0;
  state.lastBeatAgeMs = 0;
  state.maxBeatDetectCount = 0;
}

void Max30102Sensor::resetBeatState(Max30102DiagnosticState& state) {
  resetInternalBeatState();
  clearBeatTimingState(state);
  state.signalHold = false;
  state.wristSignalGate = false;
}

void Max30102Sensor::reseedAcceptedBeatClock(
    uint32_t nowMs,
    Max30102DiagnosticState& state) {
  clearAcceptedIntervalState(state);
  lastAcceptedBeatMs_ = nowMs;
  lastAcceptedIntervalMs_ = 0;
  state.acceptedBeatSeeded = true;
  state.acceptedBeatIntervalMs = 0;
  state.lastBeatIntervalMs = 0;
  state.lastBeatAgeMs = 0;
}

bool Max30102Sensor::intervalJumpAcceptable(uint32_t intervalMs) const {
  if (lastAcceptedIntervalMs_ == 0 || intervalMs == 0) {
    return true;
  }
  if (BPMWATCH_MAX30102_WRIST_MODE &&
      validIntervalCount_ < BPMWATCH_MAX30102_REQUIRED_STABLE_BEATS) {
    return true;
  }

  uint32_t baselineMs = lastAcceptedIntervalMs_;
  if (BPMWATCH_MAX30102_WRIST_MODE && validIntervalCount_ > 0) {
    const uint16_t medianMs = medianIntervalMs();
    if (medianMs != 0) {
      baselineMs = medianMs;
    }
  }
  const uint32_t diff =
      intervalMs > baselineMs ? intervalMs - baselineMs
                              : baselineMs - intervalMs;
  return (diff * 100UL / baselineMs) <=
         BPMWATCH_MAX30102_INTERVAL_JUMP_PERCENT;
}

uint16_t Max30102Sensor::medianIntervalMs() const {
  if (validIntervalCount_ == 0) {
    return 0;
  }

  uint16_t sorted[kIntervalBufferSize]{};
  for (uint8_t i = 0; i < validIntervalCount_; ++i) {
    sorted[i] = intervals_[i];
  }
  for (uint8_t i = 1; i < validIntervalCount_; ++i) {
    const uint16_t value = sorted[i];
    uint8_t j = i;
    while (j > 0 && sorted[j - 1] > value) {
      sorted[j] = sorted[j - 1];
      --j;
    }
    sorted[j] = value;
  }

  const uint8_t mid = validIntervalCount_ / 2;
  if ((validIntervalCount_ & 1U) != 0) {
    return sorted[mid];
  }
  return static_cast<uint16_t>((sorted[mid - 1] + sorted[mid]) / 2U);
}

void Max30102Sensor::pushAcceptedInterval(uint32_t intervalMs) {
  intervals_[intervalSpot_++] = static_cast<uint16_t>(intervalMs);
  intervalSpot_ %= kIntervalBufferSize;
  if (validIntervalCount_ < kIntervalBufferSize) {
    ++validIntervalCount_;
  }
}

void Max30102Sensor::rejectBeatCandidate(Max30102DiagnosticState& state,
                                         bool refractory,
                                         bool intervalOutOfRange,
                                         bool unstable) {
  ++state.rejectedBeatCount;
  ++state.falseBeatGateCount;
  if (refractory) {
    ++state.refractoryBeatRejectCount;
  }
  if (intervalOutOfRange) {
    ++state.intervalBeatRejectCount;
  }
  if (unstable) {
    ++state.unstableBeatRejectCount;
  }
  if (state.stableBeatCount < BPMWATCH_MAX30102_REQUIRED_STABLE_BEATS) {
    state.bpm = 0.0f;
    state.averageBpm = 0;
    state.bpmValid = false;
  }
}

void Max30102Sensor::sample(uint32_t nowMs,
                            Max30102DiagnosticState& state) {
  if (!state.initialized) {
    return;
  }

  const uint32_t sampleStartUs = micros();
  const auto finishSample = [&state, sampleStartUs]() {
    const uint32_t durationUs = micros() - sampleStartUs;
    state.maxSampleDurationUs = durationUs;
    if (durationUs > state.maxSampleDurationMaxUs) {
      state.maxSampleDurationMaxUs = durationUs;
    }
  };

  state.lastBeatAgeMs =
      lastAcceptedBeatMs_ == 0 ? 0 : safeAgeMs(nowMs, lastAcceptedBeatMs_);
  state.irValue = sensor_.getIR();
  state.irDcBaseline = state.irValue;
  const bool wasFingerPresent = state.fingerPresent;
  state.fingerPresent = state.irValue >= BPMWATCH_MAX30102_FINGER_IR_MIN;

  // Track per-second sample count and IR min/max
  if (state.maxSampleResetMs == 0 || nowMs - state.maxSampleResetMs >= 1000) {
    state.maxSps = state.maxSampleCount;
    state.maxSampleCount = 0;
    state.maxIrAc1s = state.maxIrMax1s - state.maxIrMin1s;
    state.maxIrMin1s = state.irValue;
    state.maxIrMax1s = state.irValue;
    state.maxSampleResetMs = nowMs;
  }
  ++state.maxSampleCount;
  if (state.irValue < state.maxIrMin1s) state.maxIrMin1s = state.irValue;
  if (state.irValue > state.maxIrMax1s) state.maxIrMax1s = state.irValue;
  state.irAcGate = state.maxIrAc1s;
  state.irAcThreshold = BPMWATCH_MAX30102_MIN_IR_AC;
  state.irAcRatioThresholdPpm = BPMWATCH_MAX30102_MIN_IR_AC_RATIO_PPM;
  state.wristSignalGate = false;
  state.wristIrAcThreshold = BPMWATCH_MAX30102_WRIST_IRAC_THR;
  state.wristIrAcExitThreshold = BPMWATCH_MAX30102_WRIST_IRAC_EXIT_THR;
  state.wristIrAcRatioThresholdPpm =
      BPMWATCH_MAX30102_WRIST_RATIO_THR_PPM;
  state.wristIrAcRatioExitPpm = BPMWATCH_MAX30102_WRIST_RATIO_EXIT_PPM;
  state.signalHold = false;
  state.signalLostMs = 0;
  state.irAcRatioPpm = 0;
  if (state.irValue > 0) {
    const uint32_t irAc =
        state.maxIrAc1s > 0 ? static_cast<uint32_t>(state.maxIrAc1s) : 0;
    state.irAcRatioPpm =
        (static_cast<uint64_t>(irAc) * 1000000ULL) /
        static_cast<uint32_t>(state.irValue);
  }

  if (!state.fingerPresent) {
    state.signalUsable = false;
    state.signalHold = false;
    state.signalLostMs = 0;
    state.signalOkReason = Max30102SignalReasonNoFinger;
    if (BPMWATCH_MAX30102_RESET_ON_NO_FINGER) {
      if (wasFingerPresent || state.bpmValid || state.averageBpm != 0 ||
          state.stableBeatCount != 0 || lastRawBeatMs_ != 0 ||
          lastAcceptedBeatMs_ != 0) {
        ++state.noFingerResetCount;
      }
      resetBeatState(state);
    } else {
      state.bpm = 0.0f;
      state.averageBpm = 0;
      state.bpmValid = false;
    }
    finishSample();
    return;
  }

  const bool samplingHealthy = state.maxSps >= BPMWATCH_MAX30102_MIN_SPS;
  const bool normalSignalGate =
      state.maxIrAc1s >= BPMWATCH_MAX30102_MIN_IR_AC &&
      state.irAcRatioPpm >= BPMWATCH_MAX30102_MIN_IR_AC_RATIO_PPM;
  const bool wristMode = BPMWATCH_MAX30102_WRIST_MODE;
  const bool wristEnterGate =
      wristMode && state.maxIrAc1s >= BPMWATCH_MAX30102_WRIST_IRAC_THR &&
      state.irAcRatioPpm >= BPMWATCH_MAX30102_WRIST_RATIO_THR_PPM;
  const bool wristExitGate =
      wristMode && state.maxIrAc1s >= BPMWATCH_MAX30102_WRIST_IRAC_EXIT_THR &&
      state.irAcRatioPpm >= BPMWATCH_MAX30102_WRIST_RATIO_EXIT_PPM;
  const bool wasSignalUsable = state.signalUsable;
  bool signalUsable = false;
  bool signalHold = false;
  bool wristSignalGate = false;
  uint8_t signalOkReason = Max30102SignalReasonLowIrAc;

  if (!samplingHealthy) {
    signalLostSinceMs_ = 0;
    signalOkReason = Max30102SignalReasonLowSps;
  } else if (normalSignalGate || wristEnterGate) {
    signalLostSinceMs_ = 0;
    signalUsable = true;
    wristSignalGate = wristEnterGate;
    if (!normalSignalGate && wristEnterGate && !wasSignalUsable) {
      ++state.wristEnterCount;
    }
    signalOkReason =
        normalSignalGate ? Max30102SignalReasonNormal
                         : Max30102SignalReasonWristEnter;
  } else if (wristMode && wasSignalUsable && wristExitGate) {
    signalLostSinceMs_ = 0;
    signalUsable = true;
    signalHold = true;
    wristSignalGate = true;
    signalOkReason = Max30102SignalReasonWristHold;
  } else if (wristMode && wasSignalUsable) {
    if (signalLostSinceMs_ == 0) {
      signalLostSinceMs_ = nowMs;
    }
    state.signalLostMs = safeAgeMs(nowMs, signalLostSinceMs_);
    if (state.signalLostMs <= BPMWATCH_MAX30102_SIGNAL_LOST_HOLD_MS) {
      signalUsable = true;
      signalHold = true;
      wristSignalGate = true;
      signalOkReason = Max30102SignalReasonHoldLost;
    } else {
      ++state.wristExitCount;
      signalOkReason = Max30102SignalReasonLost;
    }
  } else if (state.maxIrAc1s >= BPMWATCH_MAX30102_WRIST_IRAC_THR &&
             state.irAcRatioPpm < BPMWATCH_MAX30102_WRIST_RATIO_THR_PPM) {
    signalLostSinceMs_ = 0;
    signalOkReason = Max30102SignalReasonLowIrAcRatio;
  } else {
    signalLostSinceMs_ = 0;
    signalOkReason = Max30102SignalReasonLowIrAc;
  }

  state.signalUsable = signalUsable;
  state.signalHold = signalHold;
  state.wristSignalGate = wristSignalGate;
  state.signalOkReason = signalOkReason;
  if (signalLostSinceMs_ == 0 && signalUsable) {
    state.signalLostMs = 0;
  }

  if (!state.signalUsable) {
    ++state.lowSignalRejectCount;
    resetBeatState(state);
    state.signalUsable = false;
    state.signalOkReason = signalOkReason;
    finishSample();
    return;
  }

  if (!wasSignalUsable) {
    clearBeatTimingState(state);
  }
  state.acceptedBeatSeeded = lastAcceptedBeatMs_ != 0;

  if (!checkForBeat(state.irValue)) {
    finishSample();
    return;
  }

  ++state.maxBeatDetectCount;
  state.rawBeatIntervalMs =
      lastRawBeatMs_ == 0 ? 0 : safeAgeMs(nowMs, lastRawBeatMs_);
  lastRawBeatMs_ = nowMs;
  const bool acceptedBeatSeeded = lastAcceptedBeatMs_ != 0;
  const uint32_t candidateIntervalMs =
      acceptedBeatSeeded ? safeAgeMs(nowMs, lastAcceptedBeatMs_) : 0;
  state.candidateBeatIntervalMs = candidateIntervalMs;
  state.lastBeatAgeMs = candidateIntervalMs;
  state.lastBeatIntervalMs = candidateIntervalMs;

  const Max30102BeatDecision decision =
      max30102BeatCandidateDecision(acceptedBeatSeeded, candidateIntervalMs);
  if (decision == Max30102BeatDecisionSeed) {
    reseedAcceptedBeatClock(nowMs, state);
    state.candidateBeatIntervalMs = 0;
    state.rawBeatIntervalMs = 0;
    finishSample();
    return;
  }

  if (decision == Max30102BeatDecisionRefractoryReject) {
    rejectBeatCandidate(state, true, false, false);
    finishSample();
    return;
  }

  if (decision == Max30102BeatDecisionLongIntervalReseed) {
    rejectBeatCandidate(state, false, true, false);
    ++state.beatReseedCount;
    reseedAcceptedBeatClock(nowMs, state);
    state.candidateBeatIntervalMs = candidateIntervalMs;
    finishSample();
    return;
  }

  if (decision == Max30102BeatDecisionShortIntervalReject) {
    rejectBeatCandidate(state, false, true, false);
    finishSample();
    return;
  }

  if (!intervalJumpAcceptable(candidateIntervalMs)) {
    rejectBeatCandidate(state, false, false, true);
    finishSample();
    return;
  }

  pushAcceptedInterval(candidateIntervalMs);
  lastAcceptedIntervalMs_ = candidateIntervalMs;
  lastAcceptedBeatMs_ = nowMs;
  state.acceptedBeatSeeded = true;
  state.acceptedBeatIntervalMs = candidateIntervalMs;
  state.lastBeatIntervalMs = candidateIntervalMs;
  ++state.acceptedBeatCount;
  if (state.stableBeatCount < BPMWATCH_MAX30102_REQUIRED_STABLE_BEATS) {
    ++state.stableBeatCount;
  }
  state.medianBeatIntervalMs = medianIntervalMs();

  if (state.stableBeatCount < BPMWATCH_MAX30102_REQUIRED_STABLE_BEATS) {
    state.averageBpm = 0;
    state.bpmValid = false;
    finishSample();
    return;
  }

  if (state.medianBeatIntervalMs == 0) {
    state.averageBpm = 0;
    state.bpmValid = false;
    finishSample();
    return;
  }
  const float measuredBpm = 60000.0f / state.medianBeatIntervalMs;
  if (measuredBpm < BPMWATCH_MAX30102_BPM_MIN ||
      measuredBpm > BPMWATCH_MAX30102_BPM_MAX) {
    state.averageBpm = 0;
    state.bpmValid = false;
    finishSample();
    return;
  }

  state.bpm = measuredBpm;
  const uint8_t roundedBpm = static_cast<uint8_t>(measuredBpm + 0.5f);
  rates_[rateSpot_++] = roundedBpm;
  rateSpot_ %= kRateSize;
  if (validRateCount_ < kRateSize) {
    ++validRateCount_;
  }
  state.averageBpm = roundedBpm;
  state.bpmValid = true;
  finishSample();
}
#endif
