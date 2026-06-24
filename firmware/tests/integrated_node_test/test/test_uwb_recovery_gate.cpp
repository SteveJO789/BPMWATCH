#include <unity.h>

#include "../src/UwbRecoveryGate.h"

void testUwbRecoveryGateWaitsFullTimeoutBeforeRecovering() {
  UwbRecoveryGate gate;
  gate.markActivity(1000);

  TEST_ASSERT_FALSE(gate.shouldRecover(5999, 5000));
  TEST_ASSERT_TRUE(gate.shouldRecover(6000, 5000));
}

void testUwbRecoveryGateDoesNotRepeatUntilTimeoutAfterRecovery() {
  UwbRecoveryGate gate;
  gate.markActivity(1000);

  TEST_ASSERT_TRUE(gate.shouldRecover(6000, 5000));
  gate.markRecovery(6000);

  TEST_ASSERT_FALSE(gate.shouldRecover(6001, 5000));
  TEST_ASSERT_FALSE(gate.shouldRecover(10999, 5000));
  TEST_ASSERT_TRUE(gate.shouldRecover(11000, 5000));
}

void testUwbRecoveryGateActivityAfterRecoveryExtendsWindow() {
  UwbRecoveryGate gate;
  gate.markActivity(1000);

  TEST_ASSERT_TRUE(gate.shouldRecover(6000, 5000));
  gate.markRecovery(6000);
  gate.markActivity(7000);

  TEST_ASSERT_FALSE(gate.shouldRecover(11999, 5000));
  TEST_ASSERT_TRUE(gate.shouldRecover(12000, 5000));
}

void testUwbRecoveryGateObservedRangeActivitySuppressesRepeatedRecovery() {
  UwbRecoveryGate gate;
  gate.markActivity(1000);

  TEST_ASSERT_TRUE(gate.shouldRecover(16000, 15000));
  gate.markRecovery(16000);
  gate.markActivity(16500);

  TEST_ASSERT_FALSE(gate.shouldRecover(17000, 15000));
  TEST_ASSERT_FALSE(gate.shouldRecover(31000, 15000));
  TEST_ASSERT_TRUE(gate.shouldRecover(31500, 15000));
}

void testUwbRecoveryGateBlocksRepeatRecoveryEvenWhenActivityIsOld() {
  UwbRecoveryGate gate;
  gate.markActivity(1000);

  TEST_ASSERT_TRUE(gate.shouldRecover(16000, 15000));
  gate.markRecovery(16000);

  TEST_ASSERT_FALSE(gate.shouldRecover(16001, 15000));
  TEST_ASSERT_FALSE(gate.shouldRecover(30999, 15000));
  TEST_ASSERT_TRUE(gate.shouldRecover(31000, 15000));
}

void testUwbRecoveryGateReportsActivityAndRecoveryAges() {
  UwbRecoveryGate gate;
  gate.markActivity(1000);
  gate.markRecovery(6000);

  TEST_ASSERT_EQUAL_UINT32(6000, gate.lastActivityMs());
  TEST_ASSERT_EQUAL_UINT32(6000, gate.lastRecoveryMs());
  TEST_ASSERT_EQUAL_UINT32(3000, gate.activityAgeMs(9000));
  TEST_ASSERT_EQUAL_UINT32(3000, gate.recoveryAgeMs(9000));
}
