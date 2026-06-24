#include <unity.h>

#include "../src/UwbTaskCadence.h"

void testUwbTaskCadenceDefaultsZeroToOnePollPerTick() {
  TEST_ASSERT_EQUAL_UINT16(1, normalizedUwbPollsPerTick(0));
}

void testUwbTaskCadenceYieldsAfterConfiguredPolls() {
  TEST_ASSERT_FALSE(shouldYieldUwbTask(63, 64));
  TEST_ASSERT_TRUE(shouldYieldUwbTask(64, 64));
}

void testUwbTaskCadenceClampsIdlePollsToOneWhenNoIrqArrives() {
  TEST_ASSERT_EQUAL_UINT16(1, uwbPollsForTaskWake(0, 8, 8));
}

void testUwbTaskCadenceUsesBurstPollsWhenIrqArrives() {
  TEST_ASSERT_EQUAL_UINT16(8, uwbPollsForTaskWake(1, 1, 8));
  TEST_ASSERT_EQUAL_UINT16(8, uwbPollsForTaskWake(3, 1, 8));
}

void testUwbTaskCadenceClampsIrqBurstToPollsPerTick() {
  TEST_ASSERT_EQUAL_UINT16(4, uwbPollsForTaskWake(1, 1, 8, 4));
}
