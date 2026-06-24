#pragma once

#include <stdint.h>

inline uint32_t safeAgeMs(uint32_t nowMs, uint32_t lastMs) {
  if (lastMs == 0 || nowMs < lastMs) {
    return 0;
  }
  return nowMs - lastMs;
}
