#include <cassert>
#include <cmath>

#include "../src/MedianRangeFilter.h"

namespace {

bool nearlyEqual(float left, float right) {
  return std::fabs(left - right) < 0.001f;
}

void testRejectsInvalidRanges() {
  MedianRangeFilter filter;

  assert(!filter.add(-0.10f));
  assert(!filter.add(0.0f));
  assert(!filter.add(50.01f));
  assert(filter.rejectedCount() == 3);
  assert(!filter.hasValue());
}

void testMedianRejectsSingleOutlier() {
  MedianRangeFilter filter;

  assert(filter.add(10.0f));
  assert(filter.add(10.1f));
  assert(filter.add(104.25f) == false);
  assert(filter.add(9.9f));
  assert(filter.add(10.0f));
  assert(nearlyEqual(filter.value(), 10.0f));
}

void testMedianFollowsRealDistanceStep() {
  MedianRangeFilter filter;
  for (int i = 0; i < 5; ++i) {
    assert(filter.add(10.0f));
  }

  assert(filter.add(20.0f));
  assert(filter.add(20.0f));
  assert(filter.add(20.0f));
  assert(nearlyEqual(filter.value(), 20.0f));
}

}  // namespace

int main() {
  testRejectsInvalidRanges();
  testMedianRejectsSingleOutlier();
  testMedianFollowsRealDistanceStep();
  return 0;
}
