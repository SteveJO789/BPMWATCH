#pragma once

#include <stdint.h>

#include "TimeUtils.h"

class UwbRecoveryGate {
 public:
  void markActivity(uint32_t nowMs) { lastActivityMs_ = nowMs; }
  void markRecovery(uint32_t nowMs) {
    lastActivityMs_ = nowMs;
    lastRecoveryMs_ = nowMs;
    hasRecovery_ = true;
  }

  bool shouldRecover(uint32_t nowMs, uint32_t timeoutMs) const {
    const bool activityExpired = safeAgeMs(nowMs, lastActivityMs_) >= timeoutMs;
    const bool recoveryCooldownElapsed =
        !hasRecovery_ || safeAgeMs(nowMs, lastRecoveryMs_) >= timeoutMs;
    return activityExpired && recoveryCooldownElapsed;
  }

  uint32_t lastActivityMs() const { return lastActivityMs_; }
  uint32_t lastRecoveryMs() const { return lastRecoveryMs_; }
  uint32_t activityAgeMs(uint32_t nowMs) const {
    return safeAgeMs(nowMs, lastActivityMs_);
  }
  uint32_t recoveryAgeMs(uint32_t nowMs) const {
    return hasRecovery_ ? safeAgeMs(nowMs, lastRecoveryMs_) : 0;
  }

 private:
  uint32_t lastActivityMs_ = 0;
  uint32_t lastRecoveryMs_ = 0;
  bool hasRecovery_ = false;
};
