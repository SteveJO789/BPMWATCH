#include <unity.h>

#include "../src/UwbRangingDeviceReset.h"

namespace {
class FakeRangingDevices {
 public:
  explicit FakeRangingDevices(uint8_t count, bool removeShrinks = true)
      : count_(count), removeShrinks_(removeShrinks) {}

  uint8_t getNetworkDevicesNumber() const { return count_; }

  void removeNetworkDevices(int16_t index) {
    lastRemovedIndex_ = index;
    ++removeCalls_;
    if (removeShrinks_ && count_ > 0) {
      --count_;
    }
  }

  uint8_t count_ = 0;
  bool removeShrinks_ = true;
  uint8_t removeCalls_ = 0;
  int16_t lastRemovedIndex_ = -1;
};
}  // namespace

void testClearUwbRangingDevicesRemovesAllKnownPeers() {
  FakeRangingDevices devices(3);

  const uint8_t removed = clearUwbRangingDevices(devices);

  TEST_ASSERT_EQUAL_UINT8(3, removed);
  TEST_ASSERT_EQUAL_UINT8(0, devices.getNetworkDevicesNumber());
  TEST_ASSERT_EQUAL_UINT8(3, devices.removeCalls_);
  TEST_ASSERT_EQUAL_INT16(0, devices.lastRemovedIndex_);
}

void testClearUwbRangingDevicesStopsAtGuardLimit() {
  FakeRangingDevices devices(3, false);

  const uint8_t removed = clearUwbRangingDevices(devices, 2);

  TEST_ASSERT_EQUAL_UINT8(2, removed);
  TEST_ASSERT_EQUAL_UINT8(3, devices.getNetworkDevicesNumber());
  TEST_ASSERT_EQUAL_UINT8(2, devices.removeCalls_);
}
