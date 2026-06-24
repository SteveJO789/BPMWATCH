#include <unity.h>

#include "../src/Gy511Addresses.h"

void testGy511AccelAddressFallbacksCoverCommonSa0Variants() {
  TEST_ASSERT_EQUAL_HEX8(0x19, kGy511PrimaryAccelAddress);
  TEST_ASSERT_EQUAL_HEX8(0x18, kGy511FallbackAccelAddress);
  TEST_ASSERT_EQUAL_HEX8(0x1E, kGy511MagAddress);
}
