#pragma once

#include <stdint.h>

#include "FeatureFlags.h"

enum Max30102BeatDecision : uint8_t {
  Max30102BeatDecisionSeed = 0,
  Max30102BeatDecisionRefractoryReject,
  Max30102BeatDecisionShortIntervalReject,
  Max30102BeatDecisionLongIntervalReseed,
  Max30102BeatDecisionAccept
};

inline Max30102BeatDecision max30102BeatCandidateDecision(
    bool acceptedBeatSeeded,
    uint32_t candidateIntervalMs) {
  if (!acceptedBeatSeeded) {
    return Max30102BeatDecisionSeed;
  }
  if (candidateIntervalMs < BPMWATCH_MAX30102_REFRACTORY_MS) {
    return Max30102BeatDecisionRefractoryReject;
  }
  if (candidateIntervalMs < BPMWATCH_MAX30102_INTERVAL_MIN_MS) {
    return Max30102BeatDecisionShortIntervalReject;
  }
  if (candidateIntervalMs > BPMWATCH_MAX30102_INTERVAL_MAX_MS) {
    return Max30102BeatDecisionLongIntervalReseed;
  }
  return Max30102BeatDecisionAccept;
}
