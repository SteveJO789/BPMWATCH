#include <unity.h>

#include "../src/I2cScan.h"

void testI2cScanFormatsDetectedAddressesForLcd() {
  const uint8_t addresses[] = {0x1E, 0x57};
  char output[24]{};

  formatI2cAddressList(addresses, 2, output, sizeof(output));

  TEST_ASSERT_EQUAL_STRING("1E 57", output);
}

void testI2cScanShowsNoneWhenNoAddressResponds() {
  char output[24]{};

  formatI2cAddressList(nullptr, 0, output, sizeof(output));

  TEST_ASSERT_EQUAL_STRING("NONE", output);
}
