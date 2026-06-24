#pragma once

#include <stdint.h>

inline uint16_t normalizedUwbPollsPerTick(uint16_t configuredPollsPerTick) {
  return configuredPollsPerTick == 0 ? 1 : configuredPollsPerTick;
}

inline uint16_t normalizedUwbIdlePolls(uint16_t configuredIdlePolls) {
  if (configuredIdlePolls == 0) {
    return 1;
  }
  return configuredIdlePolls > 1 ? 1 : configuredIdlePolls;
}

inline bool shouldYieldUwbTask(uint16_t pollCount,
                               uint16_t configuredPollsPerTick) {
  const uint16_t pollsPerTick =
      normalizedUwbPollsPerTick(configuredPollsPerTick);
  return pollCount >= pollsPerTick;
}

inline uint16_t clampUwbPollsToTick(uint16_t pollCount,
                                    uint16_t configuredPollsPerTick) {
  const uint16_t pollsPerTick =
      normalizedUwbPollsPerTick(configuredPollsPerTick);
  return pollCount > pollsPerTick ? pollsPerTick : pollCount;
}

inline uint16_t uwbPollsForTaskWake(uint32_t notificationCount,
                                    uint16_t configuredIdlePolls,
                                    uint16_t configuredIrqPolls,
                                    uint16_t configuredPollsPerTick) {
  if (notificationCount == 0) {
    return normalizedUwbIdlePolls(configuredIdlePolls);
  }
  return clampUwbPollsToTick(normalizedUwbPollsPerTick(configuredIrqPolls),
                             configuredPollsPerTick);
}

inline uint16_t uwbPollsForTaskWake(uint32_t notificationCount,
                                    uint16_t configuredIdlePolls,
                                    uint16_t configuredIrqPolls) {
  return notificationCount > 0
             ? normalizedUwbPollsPerTick(configuredIrqPolls)
             : normalizedUwbIdlePolls(configuredIdlePolls);
}
