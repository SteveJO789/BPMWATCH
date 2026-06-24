#include "UwbDiagnostics.h"

#include <DW1000.h>
#include <SPI.h>
#include <cstring>

#include "DiagnosticsSync.h"
#include "NodeConfig.h"
#include "UwbEventQueue.h"

#ifndef UWB_ANTENNA_DELAY
#define UWB_ANTENNA_DELAY 16555
#endif

#ifndef UWB_DISCOVERY_RECOVERY_TIMEOUT_MS
#define UWB_DISCOVERY_RECOVERY_TIMEOUT_MS 5000
#endif

#ifndef UWB_CONNECTED_RECOVERY_TIMEOUT_MS
#define UWB_CONNECTED_RECOVERY_TIMEOUT_MS 15000
#endif

#ifndef BPMWATCH_TAG_UWB_RECOVERY
#define BPMWATCH_TAG_UWB_RECOVERY false
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

  uint32_t observedRangeCount = 0;
  bool observedPeerPresent = false;
  bool observedHasRange = false;
  {
    DiagnosticsLock lock;
    state.rangeStale =
        state.hasRange && (nowMs - state.lastRangeMs >= kRangeStaleMs);
    observedRangeCount = state.rangeCount;
    observedPeerPresent = state.peerPresent;
    observedHasRange = state.hasRange && !state.rangeStale;
  }

  if (observedRangeCount != lastObservedRangeCount_) {
    lastObservedRangeCount_ = observedRangeCount;
    recoveryGate_.markActivity(nowMs);
    hasEverConnected_ = true;
  }
  if (observedPeerPresent && !lastObservedPeerPresent_) {
    recoveryGate_.markActivity(nowMs);
    hasEverConnected_ = true;
  }
  lastObservedPeerPresent_ = observedPeerPresent;

  if (!kNodeConfig.isAnchor && !BPMWATCH_TAG_UWB_RECOVERY) {
    return;
  }

  if (observedHasRange) {
    return;
  }

  const uint32_t recoveryTimeoutMs =
      hasEverConnected_ ? UWB_CONNECTED_RECOVERY_TIMEOUT_MS
                        : UWB_DISCOVERY_RECOVERY_TIMEOUT_MS;
  if (recoveryGate_.shouldRecover(nowMs, recoveryTimeoutMs)) {
    uint32_t recoveryCount = 0;
    {
      DiagnosticsLock lock;
      recoveryCount = ++state.recoveryCount;
    }
    pushUwbEvent({UwbEventType::Recovery, recoveryCount, recoveryTimeoutMs, {}});
    recoveryGate_.markRecovery(nowMs);
    restart(state);
  }
}

void UwbDiagnostics::restart(UwbDiagnosticState& state) {
  {
    DiagnosticsLock lock;
    state.peerPresent = false;
  }
  lastObservedPeerPresent_ = false;
  rangeFilter_.reset();

  DW1000Ranging.initCommunication(kUwbReset, kUwbCs, kUwbIrq);
  char deviceId[64] = "not checked";
  DW1000.getPrintableDeviceIdentifier(deviceId);
  {
    DiagnosticsLock lock;
    state.spiReady = std::strncmp(deviceId, "DECA", 4) == 0;
  }
  UwbEvent event{UwbEventType::DeviceId, 0, 0, {}};
  std::strncpy(event.text, deviceId, sizeof(event.text) - 1);
  pushUwbEvent(event);

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
  recoveryGate_.markActivity(millis());
}

void UwbDiagnostics::handleNewRange() {
  DW1000Device* device = DW1000Ranging.getDistantDevice();
  if (device == nullptr || state_ == nullptr) {
    return;
  }

  const uint32_t nowMs = millis();
  const float rawDistanceM = device->getRange();
  recoveryGate_.markActivity(nowMs);
  hasEverConnected_ = true;

  if (!rangeFilter_.add(rawDistanceM)) {
    DiagnosticsLock lock;
    state_->rawDistanceM = rawDistanceM;
    state_->peerPresent = true;
    state_->rejectedCount = rangeFilter_.rejectedCount();
    return;
  }

  const float distanceM = rangeFilter_.value();
  const float rxPowerDbm = device->getRXPower();
  const float quality = device->getQuality();
  const uint32_t rejectedCount = rangeFilter_.rejectedCount();

  DiagnosticsLock lock;
  state_->rawDistanceM = rawDistanceM;
  state_->peerPresent = true;
  state_->hasRange = true;
  state_->rangeStale = false;
  state_->distanceM = distanceM;
  state_->rxPowerDbm = rxPowerDbm;
  state_->quality = quality;
  state_->lastRangeMs = nowMs;
  ++state_->rangeCount;
  state_->rejectedCount = rejectedCount;
}

void UwbDiagnostics::handleNewDevice(DW1000Device* device) {
  if (state_ == nullptr || device == nullptr) {
    return;
  }
  {
    DiagnosticsLock lock;
    state_->peerPresent = true;
  }
  hasEverConnected_ = true;
  recoveryGate_.markActivity(millis());
  pushUwbEvent({UwbEventType::PeerAdded, device->getShortAddress(), 0, {}});
}

void UwbDiagnostics::handleInactiveDevice(DW1000Device* device) {
  if (state_ != nullptr) {
    DiagnosticsLock lock;
    state_->peerPresent = false;
  }
  if (device != nullptr) {
    pushUwbEvent(
        {UwbEventType::PeerInactive, device->getShortAddress(), 0, {}});
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
