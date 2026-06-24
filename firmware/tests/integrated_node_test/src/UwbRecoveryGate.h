#pragma once

#include <stdint.h>

class UwbRecoveryGate {
 public:
  void markActivity(uint32_t nowMs) { lastActivityMs_ = nowMs; }
  void markRecovery(uint32_t nowMs) { lastActivityMs_ = nowMs; }

  bool shouldRecover(uint32_t nowMs, uint32_t timeoutMs) const {
    return nowMs - lastActivityMs_ >= timeoutMs;
  }

  uint32_t lastActivityMs() const { return lastActivityMs_; }

 private:
  uint32_t lastActivityMs_ = 0;
};
