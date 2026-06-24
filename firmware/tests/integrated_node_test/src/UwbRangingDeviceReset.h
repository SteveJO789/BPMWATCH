#pragma once

#include <stdint.h>

constexpr uint8_t kMaxUwbRangingDevicesToClear = 4;

template <typename RangingDevices>
uint8_t clearUwbRangingDevices(
    RangingDevices& devices,
    uint8_t maxRemovals = kMaxUwbRangingDevicesToClear) {
  uint8_t removed = 0;
  while (devices.getNetworkDevicesNumber() > 0 && removed < maxRemovals) {
    devices.removeNetworkDevices(0);
    ++removed;
  }
  return removed;
}
