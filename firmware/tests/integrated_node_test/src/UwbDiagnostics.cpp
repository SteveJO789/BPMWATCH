#include "UwbDiagnostics.h"

#include <DW1000.h>
#include <SPI.h>
#include <cstring>

#include "NodeConfig.h"

#ifndef UWB_ANTENNA_DELAY
#define UWB_ANTENNA_DELAY 16555
#endif

#ifndef UWB_DISCOVERY_RECOVERY_TIMEOUT_MS
#define UWB_DISCOVERY_RECOVERY_TIMEOUT_MS 5000
#endif

#ifndef UWB_CONNECTED_RECOVERY_TIMEOUT_MS
#define UWB_CONNECTED_RECOVERY_TIMEOUT_MS 15000
#endif

namespace {
constexpr int kUwbSck = 18;
constexpr int kUwbMiso = 19;
constexpr int kUwbMosi = 23;
constexpr int kUwbCs = 5;
constexpr int kUwbIrq = 34;
constexpr int kUwbReset = 4;
constexpr uint32_t kRangeStaleMs = 3000;
}  // namespace

UwbDiagnostics* UwbDiagnostics::instance_ = nullptr;
UwbDiagnosticState* UwbDiagnostics::state_ = nullptr;

void UwbDiagnostics::begin(UwbDiagnosticState& state) {
  instance_ = this;
  state_ = &state;
  std::strncpy(eui_, kNodeConfig.uwbEui, sizeof(eui_) - 1);
  SPI.begin(kUwbSck, kUwbMiso, kUwbMosi, kUwbCs);
  restart(state);
}

void UwbDiagnostics::poll(uint32_t nowMs, UwbDiagnosticState& state) {
  DW1000Ranging.loop();

  state.rangeStale =
      state.hasRange && (nowMs - state.lastRangeMs >= kRangeStaleMs);

  if (!kNodeConfig.isAnchor) {
    return;
  }

  const uint32_t recoveryTimeoutMs =
      hasEverConnected_ ? UWB_CONNECTED_RECOVERY_TIMEOUT_MS
                        : UWB_DISCOVERY_RECOVERY_TIMEOUT_MS;
  if (nowMs - lastActivityMs_ >= recoveryTimeoutMs) {
    ++state.recoveryCount;
    Serial.printf("UWB recovery #%lu after %lu ms\n",
                  static_cast<unsigned long>(state.recoveryCount),
                  static_cast<unsigned long>(recoveryTimeoutMs));
    restart(state);
  }
}

void UwbDiagnostics::restart(UwbDiagnosticState& state) {
  state.peerPresent = false;
  rangeFilter_.reset();

  DW1000Ranging.initCommunication(kUwbReset, kUwbCs, kUwbIrq);
  char deviceId[64] = "not checked";
  DW1000.getPrintableDeviceIdentifier(deviceId);
  state.spiReady = std::strncmp(deviceId, "DECA", 4) == 0;
  Serial.printf("DW1000 Device ID: %s\n", deviceId);

  DW1000.setAntennaDelay(UWB_ANTENNA_DELAY);
  DW1000Ranging.attachNewRange(onNewRange);
  DW1000Ranging.attachNewDevice(onNewDevice);
  if (kNodeConfig.isAnchor) {
    DW1000Ranging.attachBlinkDevice(onBlinkDevice);
  }
  DW1000Ranging.attachInactiveDevice(onInactiveDevice);
  DW1000Ranging.useRangeFilter(false);

  if (kNodeConfig.isAnchor) {
    DW1000Ranging.startAsAnchor(eui_, DW1000.MODE_LONGDATA_RANGE_ACCURACY,
                                false);
  } else {
    DW1000Ranging.startAsTag(eui_, DW1000.MODE_LONGDATA_RANGE_ACCURACY,
                             false);
  }
  lastActivityMs_ = millis();
}

void UwbDiagnostics::handleNewRange() {
  DW1000Device* device = DW1000Ranging.getDistantDevice();
  if (device == nullptr || state_ == nullptr) {
    return;
  }

  const uint32_t nowMs = millis();
  state_->rawDistanceM = device->getRange();
  state_->peerPresent = true;
  lastActivityMs_ = nowMs;
  hasEverConnected_ = true;

  if (!rangeFilter_.add(state_->rawDistanceM)) {
    state_->rejectedCount = rangeFilter_.rejectedCount();
    return;
  }

  state_->hasRange = true;
  state_->rangeStale = false;
  state_->distanceM = rangeFilter_.value();
  state_->rxPowerDbm = device->getRXPower();
  state_->quality = device->getQuality();
  state_->lastRangeMs = nowMs;
  ++state_->rangeCount;
  state_->rejectedCount = rangeFilter_.rejectedCount();
}

void UwbDiagnostics::handleNewDevice(DW1000Device* device) {
  if (state_ == nullptr || device == nullptr) {
    return;
  }
  state_->peerPresent = true;
  hasEverConnected_ = true;
  lastActivityMs_ = millis();
  Serial.printf("UWB peer added: 0x%04X\n", device->getShortAddress());
}

void UwbDiagnostics::handleInactiveDevice(DW1000Device* device) {
  if (state_ != nullptr) {
    state_->peerPresent = false;
  }
  if (device != nullptr) {
    Serial.printf("UWB peer inactive: 0x%04X\n", device->getShortAddress());
  }
}

void UwbDiagnostics::handleBlinkDevice(DW1000Device* device) {
  handleNewDevice(device);
}

void UwbDiagnostics::onNewRange() {
  if (instance_ != nullptr) {
    instance_->handleNewRange();
  }
}

void UwbDiagnostics::onNewDevice(DW1000Device* device) {
  if (instance_ != nullptr) {
    instance_->handleNewDevice(device);
  }
}

void UwbDiagnostics::onInactiveDevice(DW1000Device* device) {
  if (instance_ != nullptr) {
    instance_->handleInactiveDevice(device);
  }
}

void UwbDiagnostics::onBlinkDevice(DW1000Device* device) {
  if (instance_ != nullptr) {
    instance_->handleBlinkDevice(device);
  }
}

