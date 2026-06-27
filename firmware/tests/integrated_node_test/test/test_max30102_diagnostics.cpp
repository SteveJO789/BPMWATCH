#include <unity.h>

#include "../src/Max30102Diagnostics.h"

void testMax30102DiagnosticsReportsNoFingerBeforeStaleBpm() {
  TEST_ASSERT_EQUAL_STRING("NO_FINGER",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, false, false, false, 0, 100, 500, 500,
                               0, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsRequiresBpmValidForOk() {
  TEST_ASSERT_EQUAL_STRING("WAIT_STABLE",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 2, 100, 778, 6890,
                               3, 0, 0, 0, 0)));

  TEST_ASSERT_EQUAL_STRING("OK",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, true, 3, 100, 778, 6890, 4,
                               0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsLowSpsBeforeNoBeat() {
  TEST_ASSERT_EQUAL_STRING("LOW_SPS",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, false, false, 0, 18, 778, 6890,
                               0, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsLowIrAcWhenSamplingIsHealthy() {
  TEST_ASSERT_EQUAL_STRING("LOW_IRAC",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, false, false, 0, 95, 586, 4214,
                               0, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsAllowsWristLowIrAcWhenSignalUsable() {
  TEST_ASSERT_EQUAL_STRING("NO_BEAT",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 0, 100, 432, 5710,
                               0, 0, 0, 0, 0)));
}

void testMax30102SignalReasonLabelsExposeWristHysteresis() {
  TEST_ASSERT_EQUAL_STRING(
      "WRIST_ENTER",
      max30102SignalOkReasonLabel(Max30102SignalReasonWristEnter));
  TEST_ASSERT_EQUAL_STRING(
      "WRIST_HOLD",
      max30102SignalOkReasonLabel(Max30102SignalReasonWristHold));
  TEST_ASSERT_EQUAL_STRING(
      "HOLD_LOST",
      max30102SignalOkReasonLabel(Max30102SignalReasonHoldLost));
  TEST_ASSERT_EQUAL_STRING("LOST",
                           max30102SignalOkReasonLabel(
                               Max30102SignalReasonLost));
}

void testMax30102DiagnosticsReportsLowIrAcRatioAfterIrAcPasses() {
  TEST_ASSERT_EQUAL_STRING("LOW_IRAC_RATIO",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, false, false, 0, 100, 700, 4214,
                               0, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsLowSignalWhenWindowNotReady() {
  TEST_ASSERT_EQUAL_STRING("LOW_SIGNAL",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, false, false, 0, 0, 778, 6890,
                               0, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsNoBeatWhenSignalLooksUsable() {
  TEST_ASSERT_EQUAL_STRING("NO_BEAT",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 0, 100, 778, 6890,
                               0, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsUnstableBeatAfterRejectedInterval() {
  TEST_ASSERT_EQUAL_STRING("UNSTABLE_BEAT",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 3, 100, 778, 6890,
                               8, 1, 0, 0, 1)));
}

void testMax30102DiagnosticsReportsRefractoryReject() {
  TEST_ASSERT_EQUAL_STRING("REFRACTORY",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 3, 100, 778, 6890,
                               8, 1, 2, 0, 0)));
}

void testMax30102DiagnosticsReportsIntervalReject() {
  TEST_ASSERT_EQUAL_STRING("BEAT_REJECT",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 3, 100, 778, 6890,
                               8, 1, 0, 2, 0)));
}

void testMax30102DiagnosticsRejectsTableReflectionWithStaleAverage() {
  TEST_ASSERT_EQUAL_STRING("LOW_IRAC",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, false, false, 4, 100, 586, 4214,
                               16, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsFingerSignalOk() {
  TEST_ASSERT_EQUAL_STRING("OK",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, true, 3, 100, 778, 6890, 5,
                               0, 0, 0, 0)));
}

void testMax30102DiagnosticsReportsFingerWaitingStable() {
  TEST_ASSERT_EQUAL_STRING("WAIT_STABLE",
                           max30102BpmZeroReasonLabel(max30102BpmZeroReason(
                               true, true, true, false, 2, 100, 778, 6890,
                               3, 0, 0, 0, 0)));
}

void testMax30102DiagnosticsDoesNotReportOkWithoutBpmValid() {
  TEST_ASSERT_NOT_EQUAL(
      Max30102BpmReasonOk,
      max30102BpmZeroReason(true, true, true, false, 3, 100, 778, 6890, 4,
                            0, 0, 0, 0));
}
