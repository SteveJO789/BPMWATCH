#pragma once

#include <stdint.h>

inline uint16_t normalizedUwbPollsPerTick(uint16_t configuredPollsPerTick) {
  return configuredPollsPerTick == 0 ? 1 : configuredPollsPerTick;
}

inline bool shouldYieldUwbTask(uint16_t pollCount,
                               uint16_t configuredPollsPerTick) {
  const uint16_t pollsPerTick =
      normalizedUwbPollsPerTick(configuredPollsPerTick);
  return pollCount >= pollsPerTick;
}
