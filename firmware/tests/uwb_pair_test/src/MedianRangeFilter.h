#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

class MedianRangeFilter {
 public:
  static constexpr std::size_t kWindowSize = 5;
  static constexpr float kMinRangeM = 0.05f;
  static constexpr float kMaxRangeM = 50.0f;

  bool add(float rangeM) {
    if (!(rangeM >= kMinRangeM && rangeM <= kMaxRangeM)) {
      rejectedCount_++;
      return false;
    }

    samples_[nextIndex_] = rangeM;
    nextIndex_ = (nextIndex_ + 1) % kWindowSize;
    if (sampleCount_ < kWindowSize) {
      sampleCount_++;
    }
    return true;
  }

  bool hasValue() const { return sampleCount_ > 0; }

  float value() const {
    float sorted[kWindowSize]{};
    for (std::size_t i = 0; i < sampleCount_; ++i) {
      sorted[i] = samples_[i];
    }
    std::sort(sorted, sorted + sampleCount_);

    const std::size_t middle = sampleCount_ / 2;
    if (sampleCount_ % 2 == 0) {
      return (sorted[middle - 1] + sorted[middle]) * 0.5f;
    }
    return sorted[middle];
  }

  uint32_t rejectedCount() const { return rejectedCount_; }

  void reset() {
    sampleCount_ = 0;
    nextIndex_ = 0;
  }

 private:
  float samples_[kWindowSize]{};
  std::size_t sampleCount_ = 0;
  std::size_t nextIndex_ = 0;
  uint32_t rejectedCount_ = 0;
};
