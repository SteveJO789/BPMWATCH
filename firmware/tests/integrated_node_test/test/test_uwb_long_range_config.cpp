#include <unity.h>

#include "../src/UwbLongRangeConfig.h"

void testLongRangeVerifierAcceptsExpectedChannel5LongRangeSnapshot() {
  UwbLongRangeRegisterSnapshot snapshot = expectedUwbLongRangeSnapshot();

  TEST_ASSERT_EQUAL_UINT32(0, uwbLongRangeFailureMask(snapshot));
  TEST_ASSERT_EQUAL_UINT8(0, countUwbLongRangeFailures(0));
}

void testLongRangeVerifierReportsNamedFailedRegisters() {
  UwbLongRangeRegisterSnapshot snapshot = expectedUwbLongRangeSnapshot();
  snapshot.drxTune2 = 0x353B015E;
  snapshot.txPower = 0x00000000;

  const uint32_t mask = uwbLongRangeFailureMask(snapshot);
  char reason[64]{};
  formatUwbLongRangeFailureReason(mask, reason, sizeof(reason));

  TEST_ASSERT_EQUAL_UINT8(2, countUwbLongRangeFailures(mask));
  TEST_ASSERT_EQUAL_STRING("TX_POWER,DRX_TUNE2", reason);
}

void testLongRangeVerifierDoesNotRequireNssfdBitsFor110kbps() {
  UwbLongRangeRegisterSnapshot snapshot = expectedUwbLongRangeSnapshot();

  TEST_ASSERT_EQUAL_UINT32(
      0, kExpectedUwbLongRangeChanCtrl & ((1UL << 20) | (1UL << 21)));
  TEST_ASSERT_EQUAL_UINT32(0, uwbLongRangeFailureMask(snapshot));
}

void testLongRangeVerifierIgnoresRfTxctrlUpperByte() {
  UwbLongRangeRegisterSnapshot snapshot = expectedUwbLongRangeSnapshot();
  snapshot.rfTxctrl = 0xDE1E3FE0UL;

  TEST_ASSERT_EQUAL_UINT32(0, uwbLongRangeFailureMask(snapshot));
}
