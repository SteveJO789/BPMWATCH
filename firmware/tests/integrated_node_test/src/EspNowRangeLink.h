#pragma once

#include <stddef.h>
#include <stdint.h>

struct UwbDiagnosticState;
struct DiagnosticsState;

constexpr uint32_t kEspNowRangeMagic = 0x42505752UL;  // BPWR
constexpr uint8_t kEspNowRangeVersion = 2;
constexpr uint8_t kEspNowRangeLegacyVersion = 1;
constexpr uint32_t kEspNowRangeStaleMs = 3000;
constexpr uint16_t kEspNowRangeFlagRangeValid = 1U << 0;
constexpr uint16_t kEspNowRangeFlagHeadingValid = 1U << 1;
constexpr uint16_t kEspNowRangeFlagSosActive = 1U << 2;
constexpr uint16_t kEspNowRangeFlagBpmValid = 1U << 3;
constexpr uint16_t kEspNowRangeFlagBpmLost = 1U << 4;

struct EspNowRangePacket {
  uint32_t magic = kEspNowRangeMagic;
  uint8_t version = kEspNowRangeVersion;
  uint8_t senderNodeId = 0;
  uint16_t flags = 0;
  uint32_t sequence = 0;
  uint32_t rangeCount = 0;
  uint32_t sentAtMs = 0;
  float distanceM = 0.0f;
  float rawDistanceM = 0.0f;
  float rxPowerDbm = 0.0f;
  float quality = 0.0f;
  uint16_t headingCdeg = 0;
  uint16_t bpm = 0;
  uint32_t sosSeq = 0;
  uint32_t timestampMs = 0;
};

struct EspNowRangePacketV1 {
  uint32_t magic = kEspNowRangeMagic;
  uint8_t version = kEspNowRangeLegacyVersion;
  uint8_t senderNodeId = 0;
  uint16_t flags = 0;
  uint32_t sequence = 0;
  uint32_t rangeCount = 0;
  uint32_t sentAtMs = 0;
  float distanceM = 0.0f;
  float rawDistanceM = 0.0f;
  float rxPowerDbm = 0.0f;
  float quality = 0.0f;
};

inline uint16_t encodeHeadingCdeg(float headingDeg) {
  if (headingDeg < 0.0f || headingDeg >= 360.0f) {
    return 0;
  }
  const uint32_t centideg = static_cast<uint32_t>(headingDeg * 100.0f);
  return centideg > 35999U ? 35999U : static_cast<uint16_t>(centideg);
}

inline float decodeHeadingDeg(uint16_t headingCdeg) {
  return static_cast<float>(headingCdeg) / 100.0f;
}

inline uint16_t encodeBpm(int bpm) {
  if (bpm <= 0) {
    return 0;
  }
  return bpm > 255 ? 255 : static_cast<uint16_t>(bpm);
}

inline bool packetHasRange(const EspNowRangePacket& packet) {
  return (packet.flags & kEspNowRangeFlagRangeValid) != 0;
}

inline bool packetHasHeading(const EspNowRangePacket& packet) {
  return (packet.flags & kEspNowRangeFlagHeadingValid) != 0;
}

inline bool packetHasSos(const EspNowRangePacket& packet) {
  return (packet.flags & kEspNowRangeFlagSosActive) != 0;
}

inline bool packetHasBpm(const EspNowRangePacket& packet) {
  return (packet.flags & kEspNowRangeFlagBpmValid) != 0;
}

inline bool packetHasBpmLost(const EspNowRangePacket& packet) {
  return (packet.flags & kEspNowRangeFlagBpmLost) != 0;
}

inline bool packetFromLocalNode(const EspNowRangePacket& packet,
                                uint8_t localNodeId) {
  return packet.senderNodeId == localNodeId;
}

inline bool isValidEspNowRangePacket(const void* data, size_t length) {
  if (data == nullptr) {
    return false;
  }
  if (length == sizeof(EspNowRangePacket)) {
    const auto* packet = static_cast<const EspNowRangePacket*>(data);
    return packet->magic == kEspNowRangeMagic &&
           packet->version == kEspNowRangeVersion;
  }
  if (length == sizeof(EspNowRangePacketV1)) {
    const auto* packet = static_cast<const EspNowRangePacketV1*>(data);
    return packet->magic == kEspNowRangeMagic &&
           packet->version == kEspNowRangeLegacyVersion;
  }
  return false;
}

class EspNowRangeLink {
 public:
  bool begin(DiagnosticsState& diagnosticsState);
  bool sendTelemetry(const DiagnosticsState& diagnosticsState, uint32_t nowMs);
  bool ready() const { return ready_; }

 private:
  static EspNowRangeLink* instance_;
  static DiagnosticsState* diagnosticsState_;

  bool ready_ = false;
  uint32_t sequence_ = 0;
  uint8_t peerMac_[6]{};

  void handleReceive(const uint8_t* data, int length);
  void handleSendComplete(bool delivered);
  void applyPacket(const EspNowRangePacket& packet);
};
