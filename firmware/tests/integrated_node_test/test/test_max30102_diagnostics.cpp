#include <unity.h>

#include "../src/Max30102Diagnostics.h"

void testMax30102DiagnosticsReportsLowSpsBeforeNoBeat() {
  TEST_ASSERT_EQUAL_STRING("LOW_SPS",
                           max30102BpmZeroReasonLabel(
                               max30102BpmZeroReason(true, true, 0, 18, 300,
                                                     0, 0)));
}

void testMax30102DiagnosticsReportsLowIrAcWhenSamplingIsHealthy() {
  TEST_ASSERT_EQUAL_STRING("LOW_IRAC",
                           max30102BpmZeroReasonLabel(
                               max30102BpmZeroReason(true, true, 0, 95, 49,
                                                     0, 0)));
}

void testMax30102DiagnosticsReportsNoBeatWhenSignalLooksUsable() {
  TEST_ASSERT_EQUAL_STRING("NO_BEAT",
                           max30102BpmZeroReasonLabel(
                               max30102BpmZeroReason(true, true, 0, 100, 600,
                                                     0, 0)));
}
