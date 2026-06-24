#pragma once

#include <math.h>
#include <stdint.h>

struct RadarInput {
  float distanceM = 0.0f;
  bool linkOk = false;
  uint32_t timestamp = 0;
};

struct RadarState {
  float peerAngleDeg = 0.0f;
  float peerDistanceM = 0.0f;
  float lastDistanceM = 0.0f;
  float smoothedDistanceM = 0.0f;
  bool hasAngle = false;
  bool hasDistance = false;
  bool hidePeerDot = true;
  uint32_t lastUpdate = 0;
};

constexpr float kRadarMaxRangeM = 15.0f;
constexpr float kRadarDistanceDeltaThresholdM = 0.25f;
constexpr uint32_t kRadarPeerHoldMs = 3000;

inline float normalizeAngle(float angle) {
  while (angle < 0.0f) {
    angle += 360.0f;
  }
  while (angle >= 360.0f) {
    angle -= 360.0f;
  }
  return angle;
}

inline float clampFloat(float value, float minValue, float maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

inline float mapDistanceToRadius(float distanceM, float radarRadiusPx) {
  const float ratio = clampFloat(distanceM / kRadarMaxRangeM, 0.0f, 1.0f);
  const float radius = ratio * radarRadiusPx;
  return clampFloat(radius, 5.0f, radarRadiusPx);
}

inline void updateRadarState(const RadarInput& input, RadarState& state) {
  if (!input.linkOk) {
    state.hidePeerDot =
        state.lastUpdate == 0 || input.timestamp - state.lastUpdate > kRadarPeerHoldMs;
    return;
  }

  state.hidePeerDot = false;
  state.lastUpdate = input.timestamp;
  state.peerDistanceM = input.distanceM;

  if (!state.hasDistance) {
    state.lastDistanceM = input.distanceM;
    state.smoothedDistanceM = input.distanceM;
    state.peerAngleDeg = 0.0f;
    state.hasAngle = true;
    state.hasDistance = true;
    return;
  }

  state.smoothedDistanceM = state.smoothedDistanceM * 0.75f + input.distanceM * 0.25f;
  const float delta = input.distanceM - state.lastDistanceM;
  if (fabsf(delta) > kRadarDistanceDeltaThresholdM) {
    state.peerAngleDeg = delta < 0.0f ? 0.0f : 180.0f;
    state.hasAngle = true;
  }
  state.lastDistanceM = input.distanceM;
}

inline const char* radarLinkStatusLabel(bool spiReady, const RadarState& state) {
  if (!spiReady) {
    return "SPI ERR";
  }
  if (!state.hasDistance) {
    return "WAIT";
  }
  if (state.hidePeerDot) {
    return "LOST";
  }
  return "OK";
}
