#include <unity.h>

#include "../src/UwbRfQuality.h"

void testUwbRfQualityReportsGoodWhenFirstPathIsStrongAndDeltaIsSmall() {
  TEST_ASSERT_EQUAL_STRING("GOOD", uwbRfQualityLabel(-104.5f, 3.7f));
}

void testUwbRfQualityReportsWeakWhenFirstPathPowerIsLow() {
  TEST_ASSERT_EQUAL_STRING("WEAK", uwbRfQualityLabel(-111.0f, 4.0f));
}

void testUwbRfQualityReportsNlosWhenPowerDeltaIsHigh() {
  TEST_ASSERT_EQUAL_STRING("NLOS", uwbRfQualityLabel(-104.0f, 14.6f));
}
