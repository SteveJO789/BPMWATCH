#include <unity.h>

#include "../src/MedianRangeFilter.h"

void testRejectsInvalidRanges() {
  MedianRangeFilter filter;

  TEST_ASSERT_FALSE(filter.add(-0.10f));
  TEST_ASSERT_FALSE(filter.add(0.0f));
  TEST_ASSERT_FALSE(filter.add(50.01f));
  TEST_ASSERT_EQUAL_UINT32(3, filter.rejectedCount());
  TEST_ASSERT_FALSE(filter.hasValue());
}

void testMedianRejectsSingleOutlier() {
  MedianRangeFilter filter;

  TEST_ASSERT_TRUE(filter.add(10.0f));
  TEST_ASSERT_TRUE(filter.add(10.1f));
  TEST_ASSERT_FALSE(filter.add(104.25f));
  TEST_ASSERT_TRUE(filter.add(9.9f));
  TEST_ASSERT_TRUE(filter.add(10.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, filter.value());
}

void testMedianFollowsRealDistanceStep() {
  MedianRangeFilter filter;
  for (int i = 0; i < 5; ++i) {
    TEST_ASSERT_TRUE(filter.add(10.0f));
  }

  TEST_ASSERT_TRUE(filter.add(20.0f));
  TEST_ASSERT_TRUE(filter.add(20.0f));
  TEST_ASSERT_TRUE(filter.add(20.0f));
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 20.0f, filter.value());
}

void setUp() {}
void tearDown() {}

int main() {
  UNITY_BEGIN();
  RUN_TEST(testRejectsInvalidRanges);
  RUN_TEST(testMedianRejectsSingleOutlier);
  RUN_TEST(testMedianFollowsRealDistanceStep);
  return UNITY_END();
}
