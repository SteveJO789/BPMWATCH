#include <unity.h>

#include "../src/MedianRangeFilter.h"

void testGy511StatusLabelsExposeFailingI2cSide();
void testGy511InitFailureStatusesArePreservedDuringSampling();
void testGy511AccelAddressFallbacksCoverCommonSa0Variants();
void testCompassHeadingNormalizesCardinalDirections();
void testCompassHeadingAppliesHardIronOffsetAndScale();
void testCompassDiagnosticStateTracksSampleAge();
void testCompassHealthLabelUsesOkFailContract();
void testCompassModeLabelsSeparateMagOnlyFromTiltCompensated();
void testCompassRejectsRepeatedZeroMagnetometerSamples();
void testCompassDiagnosticIssueLabelsSeparateLockReadAndRawFailures();
void testCompassMagRegisterDumpLabelsHmcCompatibleDevice();
void testCompassDetectsLsm303dAtShared0x1eAddress();
void testCompassReportsPartialWhenOnlyMagHeadingIsUsable();
void testI2cScanFormatsDetectedAddressesForLcd();
void testI2cScanFormatsDeviceListForDiagnosticsLog();
void testI2cScanShowsNoneWhenNoAddressResponds();
void testRadarMapStartsWithPeerInFront();
void testRadarMapUsesFrontWhenDistanceDecreases();
void testRadarMapUsesBackWhenDistanceIncreases();
void testRadarMapKeepsAngleWhenDistanceBarelyChanges();
void testRadarMapHidesDotAfterLinkTimeout();
void testRadarMapReportsWaitBeforeFirstRange();
void testRadarMapReportsLostOnlyAfterPreviousRangeTimesOut();
void testRadarMapRotatesDemoAngleIntoNorthOrientedDisplay();
void testRadarMapKeepsDemoAngleWhenHeadingInvalid();
void testRadarBpmLostPolicyRequiresUsableBpm();
void testRadarNodeAlertUsesSosOrBpmLost();
void testRemoteBpmLostExpiresWithSnapshot();
void test_accepts_valid_packet();
void test_rejects_wrong_magic();
void test_rejects_wrong_version();
void test_rejects_wrong_size();
void test_heading_encoding_clamps_to_compact_centidegrees();
void test_bpm_encoding_clamps_to_packet_range();
void test_packet_reports_bpm_valid_and_lost_flags();
void test_packet_identifies_local_sender_for_broadcast_filtering();
void testFeatureFlagsDefaultToIntegratedDiagnostics();
void testSosButtonIgnoresShortPress();
void testSosButtonLongPressTogglesSosAndIncrementsSequence();
void testSosButtonDebouncesRawInput();
void testRemoteSosExpiresWhenPeerStopsSending();
void testUwbRecoveryGateWaitsFullTimeoutBeforeRecovering();
void testUwbRecoveryGateDoesNotRepeatUntilTimeoutAfterRecovery();
void testUwbRecoveryGateActivityAfterRecoveryExtendsWindow();
void testUwbRecoveryGateObservedRangeActivitySuppressesRepeatedRecovery();
void testUwbRecoveryGateBlocksRepeatRecoveryEvenWhenActivityIsOld();
void testUwbRecoveryGateReportsActivityAndRecoveryAges();
void testSafeAgeReturnsZeroBeforeFirstTimestamp();
void testSafeAgeReturnsZeroWhenClockWrapsBehindLastTimestamp();
void testSafeAgeReturnsElapsedMsWhenTimestampIsValid();
void testUwbTaskCadenceDefaultsZeroToOnePollPerTick();
void testUwbTaskCadenceYieldsAfterConfiguredPolls();
void testUwbTaskCadenceClampsIdlePollsToOneWhenNoIrqArrives();
void testUwbTaskCadenceUsesBurstPollsWhenIrqArrives();
void testUwbTaskCadenceClampsIrqBurstToPollsPerTick();
void testLongRangeVerifierAcceptsExpectedChannel5LongRangeSnapshot();
void testLongRangeVerifierReportsNamedFailedRegisters();
void testLongRangeVerifierDoesNotRequireNssfdBitsFor110kbps();
void testLongRangeVerifierIgnoresRfTxctrlUpperByte();
void testMax30102DiagnosticsReportsLowSpsBeforeNoBeat();
void testMax30102DiagnosticsReportsLowIrAcWhenSamplingIsHealthy();
void testMax30102DiagnosticsReportsNoBeatWhenSignalLooksUsable();
void testUwbRfQualityReportsGoodWhenFirstPathIsStrongAndDeltaIsSmall();
void testUwbRfQualityReportsWeakWhenFirstPathPowerIsLow();
void testUwbRfQualityReportsNlosWhenPowerDeltaIsHigh();
void testDw1000InactivityTimeoutKeepsAtLeastLegacyMinimum();
void testDw1000InactivityTimeoutAcceptsLongerConfiguredWindow();
void testDw1000InactivityGracePeriodIsConfigurable();
void testDw1000InactivityGracePeriodExtendsBeyondTimeout();
void testClearUwbRangingDevicesRemovesAllKnownPeers();
void testClearUwbRangingDevicesStopsAtGuardLimit();

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
  RUN_TEST(testCompassHeadingNormalizesCardinalDirections);
  RUN_TEST(testCompassHeadingAppliesHardIronOffsetAndScale);
  RUN_TEST(testCompassDiagnosticStateTracksSampleAge);
  RUN_TEST(testCompassHealthLabelUsesOkFailContract);
  RUN_TEST(testCompassModeLabelsSeparateMagOnlyFromTiltCompensated);
  RUN_TEST(testCompassRejectsRepeatedZeroMagnetometerSamples);
  RUN_TEST(testCompassDiagnosticIssueLabelsSeparateLockReadAndRawFailures);
  RUN_TEST(testCompassMagRegisterDumpLabelsHmcCompatibleDevice);
  RUN_TEST(testCompassDetectsLsm303dAtShared0x1eAddress);
  RUN_TEST(testCompassReportsPartialWhenOnlyMagHeadingIsUsable);
  RUN_TEST(testI2cScanFormatsDetectedAddressesForLcd);
  RUN_TEST(testI2cScanFormatsDeviceListForDiagnosticsLog);
  RUN_TEST(testI2cScanShowsNoneWhenNoAddressResponds);
  RUN_TEST(testRadarMapStartsWithPeerInFront);
  RUN_TEST(testRadarMapUsesFrontWhenDistanceDecreases);
  RUN_TEST(testRadarMapUsesBackWhenDistanceIncreases);
  RUN_TEST(testRadarMapKeepsAngleWhenDistanceBarelyChanges);
  RUN_TEST(testRadarMapHidesDotAfterLinkTimeout);
  RUN_TEST(testRadarMapReportsWaitBeforeFirstRange);
  RUN_TEST(testRadarMapReportsLostOnlyAfterPreviousRangeTimesOut);
  RUN_TEST(testRadarMapRotatesDemoAngleIntoNorthOrientedDisplay);
  RUN_TEST(testRadarMapKeepsDemoAngleWhenHeadingInvalid);
  RUN_TEST(testRadarBpmLostPolicyRequiresUsableBpm);
  RUN_TEST(testRadarNodeAlertUsesSosOrBpmLost);
  RUN_TEST(testRemoteBpmLostExpiresWithSnapshot);
  RUN_TEST(test_accepts_valid_packet);
  RUN_TEST(test_rejects_wrong_magic);
  RUN_TEST(test_rejects_wrong_version);
  RUN_TEST(test_rejects_wrong_size);
  RUN_TEST(test_heading_encoding_clamps_to_compact_centidegrees);
  RUN_TEST(test_bpm_encoding_clamps_to_packet_range);
  RUN_TEST(test_packet_reports_bpm_valid_and_lost_flags);
  RUN_TEST(test_packet_identifies_local_sender_for_broadcast_filtering);
  RUN_TEST(testFeatureFlagsDefaultToIntegratedDiagnostics);
  RUN_TEST(testSosButtonIgnoresShortPress);
  RUN_TEST(testSosButtonLongPressTogglesSosAndIncrementsSequence);
  RUN_TEST(testSosButtonDebouncesRawInput);
  RUN_TEST(testRemoteSosExpiresWhenPeerStopsSending);
  RUN_TEST(testUwbRecoveryGateWaitsFullTimeoutBeforeRecovering);
  RUN_TEST(testUwbRecoveryGateDoesNotRepeatUntilTimeoutAfterRecovery);
  RUN_TEST(testUwbRecoveryGateActivityAfterRecoveryExtendsWindow);
  RUN_TEST(testUwbRecoveryGateObservedRangeActivitySuppressesRepeatedRecovery);
  RUN_TEST(testUwbRecoveryGateBlocksRepeatRecoveryEvenWhenActivityIsOld);
  RUN_TEST(testUwbRecoveryGateReportsActivityAndRecoveryAges);
  RUN_TEST(testSafeAgeReturnsZeroBeforeFirstTimestamp);
  RUN_TEST(testSafeAgeReturnsZeroWhenClockWrapsBehindLastTimestamp);
  RUN_TEST(testSafeAgeReturnsElapsedMsWhenTimestampIsValid);
  RUN_TEST(testUwbTaskCadenceDefaultsZeroToOnePollPerTick);
  RUN_TEST(testUwbTaskCadenceYieldsAfterConfiguredPolls);
  RUN_TEST(testUwbTaskCadenceClampsIdlePollsToOneWhenNoIrqArrives);
  RUN_TEST(testUwbTaskCadenceUsesBurstPollsWhenIrqArrives);
  RUN_TEST(testUwbTaskCadenceClampsIrqBurstToPollsPerTick);
  RUN_TEST(testLongRangeVerifierAcceptsExpectedChannel5LongRangeSnapshot);
  RUN_TEST(testLongRangeVerifierReportsNamedFailedRegisters);
  RUN_TEST(testLongRangeVerifierDoesNotRequireNssfdBitsFor110kbps);
  RUN_TEST(testLongRangeVerifierIgnoresRfTxctrlUpperByte);
  RUN_TEST(testMax30102DiagnosticsReportsLowSpsBeforeNoBeat);
  RUN_TEST(testMax30102DiagnosticsReportsLowIrAcWhenSamplingIsHealthy);
  RUN_TEST(testMax30102DiagnosticsReportsNoBeatWhenSignalLooksUsable);
  RUN_TEST(testUwbRfQualityReportsGoodWhenFirstPathIsStrongAndDeltaIsSmall);
  RUN_TEST(testUwbRfQualityReportsWeakWhenFirstPathPowerIsLow);
  RUN_TEST(testUwbRfQualityReportsNlosWhenPowerDeltaIsHigh);
  RUN_TEST(testDw1000InactivityTimeoutKeepsAtLeastLegacyMinimum);
  RUN_TEST(testDw1000InactivityTimeoutAcceptsLongerConfiguredWindow);
  RUN_TEST(testDw1000InactivityGracePeriodIsConfigurable);
  RUN_TEST(testDw1000InactivityGracePeriodExtendsBeyondTimeout);
  RUN_TEST(testClearUwbRangingDevicesRemovesAllKnownPeers);
  RUN_TEST(testClearUwbRangingDevicesStopsAtGuardLimit);
  return UNITY_END();
}
