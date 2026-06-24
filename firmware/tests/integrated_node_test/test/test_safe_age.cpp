#include <unity.h>

#include "../src/TimeUtils.h"

void testSafeAgeReturnsZeroBeforeFirstTimestamp() {
  TEST_ASSERT_EQUAL_UINT32(0, safeAgeMs(1000, 0));
}

void testSafeAgeReturnsZeroWhenClockWrapsBehindLastTimestamp() {
  TEST_ASSERT_EQUAL_UINT32(0, safeAgeMs(10, 20));
}

void testSafeAgeReturnsElapsedMsWhenTimestampIsValid() {
  TEST_ASSERT_EQUAL_UINT32(250, safeAgeMs(1250, 1000));
}
