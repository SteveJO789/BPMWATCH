#include "FeatureFlags.h"

#if BPMWATCH_ESPNOW_RANGE_LINK
#include "EspNowRangeLink.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "DiagnosticsSync.h"
#include "DiagnosticsState.h"
#include "NodeConfig.h"

#include <WiFi.h>
#include <esp_now.h>

namespace {
constexpr uint8_t kNodeBPeerMac[6] = {0x1C, 0x75, 0xC4, 0xF4, 0xE9, 0xD4};
}
#endif

EspNowRangeLink* EspNowRangeLink::instance_ = nullptr;
UwbDiagnosticState* EspNowRangeLink::uwbState_ = nullptr;

bool EspNowRangeLink::begin(UwbDiagnosticState& uwbState) {
  uwbState_ = &uwbState;
  instance_ = this;

#if defined(ARDUINO_ARCH_ESP32)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);

  if (esp_now_init() != ESP_OK) {
    ready_ = false;
    DiagnosticsLock lock;
    uwbState.espNowReady = false;
    return false;
  }

  if (kNodeConfig.isAnchor) {
    esp_now_peer_info_t peerInfo{};
    memcpy(peerInfo.peer_addr, kNodeBPeerMac, sizeof(kNodeBPeerMac));
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (!esp_now_is_peer_exist(kNodeBPeerMac) &&
        esp_now_add_peer(&peerInfo) != ESP_OK) {
      ready_ = false;
      DiagnosticsLock lock;
      uwbState.espNowReady = false;
      return false;
    }
  } else {
    esp_now_register_recv_cb(
        [](const esp_now_recv_info_t*, const uint8_t* data, int length) {
          if (instance_ != nullptr) {
            instance_->handleReceive(data, length);
          }
        });
  }

  ready_ = true;
  {
    DiagnosticsLock lock;
    uwbState.espNowReady = true;
  }
  return true;
#else
  (void)uwbState;
  ready_ = false;
  return false;
#endif
}

bool EspNowRangeLink::sendRange(const UwbDiagnosticState& uwbState,
                                uint32_t nowMs) {
#if defined(ARDUINO_ARCH_ESP32)
  if (!ready_ || !kNodeConfig.isAnchor || !uwbState.hasRange) {
    return false;
  }

  EspNowRangePacket packet{};
  packet.senderNodeId = kNodeConfig.nodeId;
  packet.sequence = ++sequence_;
  packet.rangeCount = uwbState.rangeCount;
  packet.sentAtMs = nowMs;
  packet.distanceM = uwbState.distanceM;
  packet.rawDistanceM = uwbState.rawDistanceM;
  packet.rxPowerDbm = uwbState.rxPowerDbm;
  packet.quality = uwbState.quality;

  if (uwbState_ != nullptr) {
    DiagnosticsLock lock;
    ++uwbState_->espNowTxCount;
  }
  const esp_err_t result =
      esp_now_send(kNodeBPeerMac, reinterpret_cast<const uint8_t*>(&packet),
                   sizeof(packet));
  if (result != ESP_OK && uwbState_ != nullptr) {
    DiagnosticsLock lock;
    ++uwbState_->espNowTxFailCount;
  }
  return result == ESP_OK;
#else
  (void)uwbState;
  (void)nowMs;
  return false;
#endif
}

void EspNowRangeLink::handleReceive(const uint8_t* data, int length) {
  if (!isValidEspNowRangePacket(data, static_cast<size_t>(length))) {
    return;
  }
  const auto* packet = reinterpret_cast<const EspNowRangePacket*>(data);
  applyPacket(*packet);
}

void EspNowRangeLink::applyPacket(const EspNowRangePacket& packet) {
#if defined(ARDUINO_ARCH_ESP32)
  if (uwbState_ == nullptr) {
    return;
  }
  const uint32_t nowMs = millis();
  DiagnosticsLock lock;
  uwbState_->peerPresent = true;
  uwbState_->hasRange = true;
  uwbState_->rangeStale = false;
  uwbState_->distanceM = packet.distanceM;
  uwbState_->rawDistanceM = packet.rawDistanceM;
  uwbState_->rxPowerDbm = packet.rxPowerDbm;
  uwbState_->quality = packet.quality;
  uwbState_->lastRangeMs = nowMs;
  uwbState_->rangeCount = packet.rangeCount;
  uwbState_->espNowRxCount++;
  uwbState_->lastEspNowRxMs = nowMs;
#else
  (void)packet;
#endif
}
#endif
