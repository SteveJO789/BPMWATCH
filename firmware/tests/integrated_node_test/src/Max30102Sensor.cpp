#include "FeatureFlags.h"

#if BPMWATCH_ENABLE_MAX30102
#include "Max30102Sensor.h"

#include <heartRate.h>

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
  rateSpot_ = 0;
  validRateCount_ = 0;
  lastBeatMs_ = 0;
  for (uint8_t& rate : rates_) {
    rate = 0;
  }
  return true;
}

void Max30102Sensor::sample(uint32_t nowMs,
                            Max30102DiagnosticState& state) {
  if (!state.initialized) {
    return;
  }

  state.lastBeatAgeMs = safeAgeMs(nowMs, lastBeatMs_);
  state.irValue = sensor_.getIR();
  state.fingerPresent = state.irValue >= 30000;

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

  if (!checkForBeat(state.irValue)) {
    return;
  }

  ++state.maxBeatDetectCount;

  if (lastBeatMs_ > 0) {
    const uint32_t beatIntervalMs = safeAgeMs(nowMs, lastBeatMs_);
    state.lastBeatAgeMs = beatIntervalMs;
    state.lastBeatIntervalMs = beatIntervalMs;
    if (beatIntervalMs > 0) {
      const float measuredBpm = 60000.0f / beatIntervalMs;
      if (measuredBpm >= 40.0f && measuredBpm <= 220.0f) {
        state.bpm = measuredBpm;
        rates_[rateSpot_++] = static_cast<uint8_t>(measuredBpm);
        rateSpot_ %= kRateSize;
        if (validRateCount_ < kRateSize) {
          ++validRateCount_;
        }

        int total = 0;
        for (uint8_t i = 0; i < validRateCount_; ++i) {
          total += rates_[i];
        }
        state.averageBpm = total / validRateCount_;
      } else {
        ++state.rejectedBeatCount;
      }
    }
  }
  lastBeatMs_ = nowMs;
}
#endif
