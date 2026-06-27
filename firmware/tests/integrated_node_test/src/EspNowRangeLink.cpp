#include "FeatureFlags.h"

#if BPMWATCH_ESPNOW_RANGE_LINK
#include "EspNowRangeLink.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "DiagnosticsSync.h"
#include "DiagnosticsState.h"
#include "NodeConfig.h"

#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

#include <stdio.h>

#ifndef BPMWATCH_ESPNOW_BROADCAST
#define BPMWATCH_ESPNOW_BROADCAST true
#endif

#ifndef BPMWATCH_ESPNOW_CHANNEL
#define BPMWATCH_ESPNOW_CHANNEL 1
#endif

namespace {
constexpr uint8_t kNodeAPeerMac[6] = {0x0C, 0x8A, 0xD3, 0x7C, 0xE5, 0xA4};
constexpr uint8_t kNodeBPeerMac[6] = {0x1C, 0x75, 0xC4, 0xF4, 0xE9, 0xD4};
constexpr uint8_t kBroadcastPeerMac[6] = {0xFF, 0xFF, 0xFF,
                                          0xFF, 0xFF, 0xFF};

const uint8_t* peerMacForCurrentNode() {
#if BPMWATCH_ESPNOW_BROADCAST
  return kBroadcastPeerMac;
#else
  return kNodeConfig.isAnchor ? kNodeBPeerMac : kNodeAPeerMac;
#endif
}

void formatMac(const uint8_t* mac, char* out, size_t outSize) {
  if (outSize < 18) {
    return;
  }
  snprintf(out, outSize, "%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1],
           mac[2], mac[3], mac[4], mac[5]);
}
}
#endif

EspNowRangeLink* EspNowRangeLink::instance_ = nullptr;
DiagnosticsState* EspNowRangeLink::diagnosticsState_ = nullptr;

bool EspNowRangeLink::begin(DiagnosticsState& diagnosticsState) {
  diagnosticsState_ = &diagnosticsState;
  instance_ = this;

#if defined(ARDUINO_ARCH_ESP32)
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(false, false);
  esp_wifi_set_channel(BPMWATCH_ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    ready_ = false;
    DiagnosticsLock lock;
    diagnosticsState.uwb.espNowReady = false;
    return false;
  }

  memcpy(peerMac_, peerMacForCurrentNode(), sizeof(peerMac_));
  esp_now_peer_info_t peerInfo{};
  memcpy(peerInfo.peer_addr, peerMac_, sizeof(peerMac_));
  peerInfo.channel = BPMWATCH_ESPNOW_CHANNEL;
  peerInfo.encrypt = false;
  if (!esp_now_is_peer_exist(peerMac_) &&
      esp_now_add_peer(&peerInfo) != ESP_OK) {
    ready_ = false;
    DiagnosticsLock lock;
    diagnosticsState.uwb.espNowReady = false;
    return false;
  }

  esp_now_register_recv_cb(
      [](const esp_now_recv_info_t*, const uint8_t* data, int length) {
        if (instance_ != nullptr) {
          instance_->handleReceive(data, length);
        }
      });
  esp_now_register_send_cb(
      [](const wifi_tx_info_t*, esp_now_send_status_t status) {
        if (instance_ != nullptr) {
          instance_->handleSendComplete(status == ESP_NOW_SEND_SUCCESS);
        }
      });

  ready_ = true;
  {
    DiagnosticsLock lock;
    diagnosticsState.uwb.espNowReady = true;
  }
  char peerMacText[18]{};
  formatMac(peerMac_, peerMacText, sizeof(peerMacText));
  Serial.printf("ESPNOW config: local_sta=%s peer=%s channel=%d broadcast=%d\n",
                WiFi.macAddress().c_str(), peerMacText,
                BPMWATCH_ESPNOW_CHANNEL, BPMWATCH_ESPNOW_BROADCAST ? 1 : 0);
  return true;
#else
  (void)diagnosticsState;
  ready_ = false;
  return false;
#endif
}

bool EspNowRangeLink::sendTelemetry(const DiagnosticsState& diagnosticsState,
                                    uint32_t nowMs) {
#if defined(ARDUINO_ARCH_ESP32)
  if (!ready_) {
    return false;
  }

  EspNowRangePacket packet{};
  packet.senderNodeId = kNodeConfig.nodeId;
  packet.sequence = ++sequence_;
  packet.rangeCount = diagnosticsState.uwb.rangeCount;
  packet.sentAtMs = nowMs;
  packet.timestampMs = nowMs;
  packet.sosSeq = diagnosticsState.sos.sosSeq;
  if (diagnosticsState.uwb.hasRange && !diagnosticsState.uwb.rangeStale) {
    packet.flags |= kEspNowRangeFlagRangeValid;
    packet.distanceM = diagnosticsState.uwb.distanceM;
    packet.rawDistanceM = diagnosticsState.uwb.rawDistanceM;
    packet.rxPowerDbm = diagnosticsState.uwb.rxPowerDbm;
    packet.quality = diagnosticsState.uwb.quality;
  }
  if (compassHeadingValid(diagnosticsState.compass)) {
    packet.flags |= kEspNowRangeFlagHeadingValid;
    packet.headingCdeg = encodeHeadingCdeg(diagnosticsState.compass.headingDeg);
  }
#if BPMWATCH_ENABLE_MAX30102
  if (radarBpmLost(true, diagnosticsState.max30102.initialized,
                   diagnosticsState.max30102.fingerPresent,
                   diagnosticsState.max30102.averageBpm)) {
    packet.flags |= kEspNowRangeFlagBpmLost;
  } else {
    packet.flags |= kEspNowRangeFlagBpmValid;
    packet.bpm = encodeBpm(diagnosticsState.max30102.averageBpm);
  }
#endif
  if (diagnosticsState.sos.sosActive) {
    packet.flags |= kEspNowRangeFlagSosActive;
  }

  if (diagnosticsState_ != nullptr) {
    DiagnosticsLock lock;
    ++diagnosticsState_->uwb.espNowTxCount;
  }
  const esp_err_t result =
      esp_now_send(peerMac_, reinterpret_cast<const uint8_t*>(&packet),
                   sizeof(packet));
  if (result != ESP_OK && diagnosticsState_ != nullptr) {
    DiagnosticsLock lock;
    ++diagnosticsState_->uwb.espNowTxFailCount;
  }
  return result == ESP_OK;
#else
  (void)diagnosticsState;
  (void)nowMs;
  return false;
#endif
}

void EspNowRangeLink::handleSendComplete(bool delivered) {
#if defined(ARDUINO_ARCH_ESP32)
  if (delivered || diagnosticsState_ == nullptr) {
    return;
  }
  DiagnosticsLock lock;
  ++diagnosticsState_->uwb.espNowTxFailCount;
#else
  (void)delivered;
#endif
}

void EspNowRangeLink::handleReceive(const uint8_t* data, int length) {
  if (!isValidEspNowRangePacket(data, static_cast<size_t>(length))) {
    return;
  }
  if (static_cast<size_t>(length) == sizeof(EspNowRangePacketV1)) {
    const auto* legacy = reinterpret_cast<const EspNowRangePacketV1*>(data);
    EspNowRangePacket packet{};
    packet.senderNodeId = legacy->senderNodeId;
    packet.flags = kEspNowRangeFlagRangeValid;
    packet.sequence = legacy->sequence;
    packet.rangeCount = legacy->rangeCount;
    packet.sentAtMs = legacy->sentAtMs;
    packet.timestampMs = legacy->sentAtMs;
    packet.distanceM = legacy->distanceM;
    packet.rawDistanceM = legacy->rawDistanceM;
    packet.rxPowerDbm = legacy->rxPowerDbm;
    packet.quality = legacy->quality;
    if (packetFromLocalNode(packet, kNodeConfig.nodeId)) {
      return;
    }
    applyPacket(packet);
    return;
  }
  const auto* packet = reinterpret_cast<const EspNowRangePacket*>(data);
  if (packetFromLocalNode(*packet, kNodeConfig.nodeId)) {
    return;
  }
  applyPacket(*packet);
}

void EspNowRangeLink::applyPacket(const EspNowRangePacket& packet) {
#if defined(ARDUINO_ARCH_ESP32)
  if (diagnosticsState_ == nullptr) {
    return;
  }
  const uint32_t nowMs = millis();
  DiagnosticsLock lock;
  diagnosticsState_->peer.nodeId = packet.senderNodeId;
  diagnosticsState_->peer.lastRxMs = nowMs;
  if (packetHasHeading(packet)) {
    diagnosticsState_->peer.headingValid = true;
    diagnosticsState_->peer.headingDeg = decodeHeadingDeg(packet.headingCdeg);
  } else {
    diagnosticsState_->peer.headingValid = false;
  }
  diagnosticsState_->peer.bpmValid = packetHasBpm(packet);
  diagnosticsState_->peer.bpmLost = packetHasBpmLost(packet);
  diagnosticsState_->peer.bpm =
      diagnosticsState_->peer.bpmValid ? packet.bpm : 0;
  if (diagnosticsState_->peer.bpmValid || diagnosticsState_->peer.bpmLost) {
    diagnosticsState_->peer.lastBpmRxMs = nowMs;
  }
  diagnosticsState_->peer.remoteSos.active = packetHasSos(packet);
  diagnosticsState_->peer.remoteSos.seq = packet.sosSeq;
  diagnosticsState_->peer.remoteSos.lastRxMs = nowMs;
  diagnosticsState_->peer.remoteSos.senderNodeId = packet.senderNodeId;

  if (packetHasRange(packet)) {
    diagnosticsState_->uwb.peerPresent = true;
    diagnosticsState_->uwb.hasRange = true;
    diagnosticsState_->uwb.rangeStale = false;
    diagnosticsState_->uwb.distanceM = packet.distanceM;
    diagnosticsState_->uwb.rawDistanceM = packet.rawDistanceM;
    diagnosticsState_->uwb.rxPowerDbm = packet.rxPowerDbm;
    diagnosticsState_->uwb.quality = packet.quality;
    diagnosticsState_->uwb.lastRangeMs = nowMs;
    diagnosticsState_->uwb.rangeCount = packet.rangeCount;
  }
  diagnosticsState_->uwb.espNowRxCount++;
  diagnosticsState_->uwb.lastEspNowRxMs = nowMs;
#else
  (void)packet;
#endif
}
#endif
