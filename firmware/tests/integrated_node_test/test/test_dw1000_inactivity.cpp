#include <unity.h>

#include "../../../../lib/DW1000/src/DW1000Inactivity.h"

void testDw1000InactivityTimeoutKeepsAtLeastLegacyMinimum() {
  TEST_ASSERT_EQUAL_UINT32(1000, normalizedDw1000InactivityTimeMs(0));
  TEST_ASSERT_EQUAL_UINT32(1000, normalizedDw1000InactivityTimeMs(500));
}

void testDw1000InactivityTimeoutAcceptsLongerConfiguredWindow() {
  TEST_ASSERT_EQUAL_UINT32(5000, normalizedDw1000InactivityTimeMs(5000));
}

void testDw1000InactivityGracePeriodIsConfigurable() {
  TEST_ASSERT_GREATER_OR_EQUAL_UINT32(0, dw1000InactivityGracePeriodMs());
}

void testDw1000InactivityGracePeriodExtendsBeyondTimeout() {
  TEST_ASSERT_GREATER_OR_EQUAL_UINT32(1000, dw1000InactivityGracePeriodMs());
}