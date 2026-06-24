#include <unity.h>

#include "../src/UwbTaskCadence.h"

void testUwbTaskCadenceDefaultsZeroToOnePollPerTick() {
  TEST_ASSERT_EQUAL_UINT16(1, normalizedUwbPollsPerTick(0));
}

void testUwbTaskCadenceYieldsAfterConfiguredPolls() {
  TEST_ASSERT_FALSE(shouldYieldUwbTask(63, 64));
  TEST_ASSERT_TRUE(shouldYieldUwbTask(64, 64));
}
