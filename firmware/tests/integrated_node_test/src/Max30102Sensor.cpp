#include "FeatureFlags.h"

#if BPMWATCH_ENABLE_MAX30102
#include "Max30102Sensor.h"

#include <heartRate.h>

bool Max30102Sensor::begin(TwoWire& wire) {
  if (!sensor_.begin(wire, I2C_SPEED_STANDARD)) {
    return false;
  }
  sensor_.setup();
  sensor_.setPulseAmplitudeRed(0x0A);
  sensor_.setPulseAmplitudeGreen(0);
  return true;
}

void Max30102Sensor::sample(uint32_t nowMs,
                            Max30102DiagnosticState& state) {
  if (!state.initialized) {
    return;
  }

  state.irValue = sensor_.getIR();
  state.fingerPresent = state.irValue >= 50000;
  if (!checkForBeat(state.irValue)) {
    return;
  }

  if (lastBeatMs_ > 0) {
    const uint32_t beatIntervalMs = nowMs - lastBeatMs_;
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
      }
    }
  }
  lastBeatMs_ = nowMs;
}
#endif
