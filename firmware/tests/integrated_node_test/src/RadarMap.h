#pragma once

#include <math.h>
#include <stdint.h>

#include "TimeUtils.h"

struct RadarInput {
  float distanceM = 0.0f;
  bool linkOk = false;
  uint32_t timestamp = 0;
};

struct RadarState {
  // Body-relative/movement-based demo angle. This is not true peer bearing.
  float demoRadarAngleDeg = 0.0f;
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
constexpr uint32_t kRemoteBpmLostHoldMs = 3000;

#ifndef BPMWATCH_BPM_LOST_ALERT_GRACE_MS
#define BPMWATCH_BPM_LOST_ALERT_GRACE_MS 300000UL
#endif

constexpr uint32_t kBpmLostAlertGraceMs = BPMWATCH_BPM_LOST_ALERT_GRACE_MS;

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

inline float northOrientedRadarAngleDeg(float demoRadarAngleDeg,
                                        float localHeadingDeg,
                                        bool headingValid) {
  if (!headingValid) {
    return normalizeAngle(demoRadarAngleDeg);
  }
  return normalizeAngle(demoRadarAngleDeg + localHeadingDeg);
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
    state.demoRadarAngleDeg = 0.0f;
    state.peerAngleDeg = state.demoRadarAngleDeg;
    state.hasAngle = true;
    state.hasDistance = true;
    return;
  }

  state.smoothedDistanceM = state.smoothedDistanceM * 0.75f + input.distanceM * 0.25f;
  const float delta = input.distanceM - state.lastDistanceM;
  if (fabsf(delta) > kRadarDistanceDeltaThresholdM) {
    state.demoRadarAngleDeg = delta < 0.0f ? 0.0f : 180.0f;
    state.peerAngleDeg = state.demoRadarAngleDeg;
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

inline bool radarBpmLost(bool sensorEnabled,
                         bool initialized,
                         bool fingerPresent,
                         bool signalUsable,
                         bool bpmValid) {
  if (!sensorEnabled) {
    return false;
  }
  return !initialized || !fingerPresent || !signalUsable || !bpmValid;
}

inline void updateRadarBpmLostAlert(bool sensorEnabled,
                                    bool bpmUsable,
                                    uint32_t nowMs,
                                    uint32_t& invalidSinceMs,
                                    uint32_t& invalidAgeMs,
                                    bool& alert) {
  if (!sensorEnabled || bpmUsable) {
    invalidSinceMs = 0;
    invalidAgeMs = 0;
    alert = false;
    return;
  }

  if (invalidSinceMs == 0) {
    invalidSinceMs = nowMs == 0 ? 1 : nowMs;
    invalidAgeMs = 0;
    alert = false;
    return;
  }

  invalidAgeMs = safeAgeMs(nowMs, invalidSinceMs);
  alert = invalidAgeMs >= kBpmLostAlertGraceMs;
}

inline bool radarNodeAlert(bool sosActive, bool bpmLost) {
  return sosActive || bpmLost;
}

inline bool remoteBpmLostVisible(bool bpmLost, uint32_t lastRxMs, uint32_t nowMs) {
  return bpmLost && lastRxMs != 0 &&
         safeAgeMs(nowMs, lastRxMs) <= kRemoteBpmLostHoldMs;
}
