#include <unity.h>

#include "../src/MedianRangeFilter.h"

void testGy511StatusLabelsExposeFailingI2cSide();
void testGy511InitFailureStatusesArePreservedDuringSampling();
void testGy511AccelAddressFallbacksCoverCommonSa0Variants();
void testI2cScanFormatsDetectedAddressesForLcd();
void testI2cScanShowsNoneWhenNoAddressResponds();
void testRadarMapStartsWithPeerInFront();
void testRadarMapUsesFrontWhenDistanceDecreases();
void testRadarMapUsesBackWhenDistanceIncreases();
void testRadarMapKeepsAngleWhenDistanceBarelyChanges();
void testRadarMapHidesDotAfterLinkTimeout();
void testRadarMapReportsWaitBeforeFirstRange();
void testRadarMapReportsLostOnlyAfterPreviousRangeTimesOut();
void test_accepts_valid_packet();
void test_rejects_wrong_magic();
void test_rejects_wrong_version();
void test_rejects_wrong_size();
void testFeatureFlagsDefaultToIntegratedDiagnostics();
void testUwbRecoveryGateWaitsFullTimeoutBeforeRecovering();
void testUwbRecoveryGateDoesNotRepeatUntilTimeoutAfterRecovery();
void testUwbRecoveryGateActivityAfterRecoveryExtendsWindow();
void testUwbRecoveryGateObservedRangeActivitySuppressesRepeatedRecovery();
void testUwbTaskCadenceDefaultsZeroToOnePollPerTick();
void testUwbTaskCadenceYieldsAfterConfiguredPolls();

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
  RUN_TEST(testGy511StatusLabelsExposeFailingI2cSide);
  RUN_TEST(testGy511InitFailureStatusesArePreservedDuringSampling);
  RUN_TEST(testGy511AccelAddressFallbacksCoverCommonSa0Variants);
  RUN_TEST(testI2cScanFormatsDetectedAddressesForLcd);
  RUN_TEST(testI2cScanShowsNoneWhenNoAddressResponds);
  RUN_TEST(testRadarMapStartsWithPeerInFront);
  RUN_TEST(testRadarMapUsesFrontWhenDistanceDecreases);
  RUN_TEST(testRadarMapUsesBackWhenDistanceIncreases);
  RUN_TEST(testRadarMapKeepsAngleWhenDistanceBarelyChanges);
  RUN_TEST(testRadarMapHidesDotAfterLinkTimeout);
  RUN_TEST(testRadarMapReportsWaitBeforeFirstRange);
  RUN_TEST(testRadarMapReportsLostOnlyAfterPreviousRangeTimesOut);
  RUN_TEST(test_accepts_valid_packet);
  RUN_TEST(test_rejects_wrong_magic);
  RUN_TEST(test_rejects_wrong_version);
  RUN_TEST(test_rejects_wrong_size);
  RUN_TEST(testFeatureFlagsDefaultToIntegratedDiagnostics);
  RUN_TEST(testUwbRecoveryGateWaitsFullTimeoutBeforeRecovering);
  RUN_TEST(testUwbRecoveryGateDoesNotRepeatUntilTimeoutAfterRecovery);
  RUN_TEST(testUwbRecoveryGateActivityAfterRecoveryExtendsWindow);
  RUN_TEST(testUwbRecoveryGateObservedRangeActivitySuppressesRepeatedRecovery);
  RUN_TEST(testUwbTaskCadenceDefaultsZeroToOnePollPerTick);
  RUN_TEST(testUwbTaskCadenceYieldsAfterConfiguredPolls);
  return UNITY_END();
}
