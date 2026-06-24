#pragma once

#include <stddef.h>
#include <stdint.h>

struct UwbDiagnosticState;

constexpr uint32_t kEspNowRangeMagic = 0x42505752UL;  // BPWR
constexpr uint8_t kEspNowRangeVersion = 1;
constexpr uint32_t kEspNowRangeStaleMs = 3000;

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
};

inline bool isValidEspNowRangePacket(const void* data, size_t length) {
  if (data == nullptr || length != sizeof(EspNowRangePacket)) {
    return false;
  }
  const auto* packet = static_cast<const EspNowRangePacket*>(data);
  return packet->magic == kEspNowRangeMagic &&
         packet->version == kEspNowRangeVersion;
}

class EspNowRangeLink {
 public:
  bool begin(UwbDiagnosticState& uwbState);
  bool sendRange(const UwbDiagnosticState& uwbState, uint32_t nowMs);
  bool ready() const { return ready_; }

 private:
  static EspNowRangeLink* instance_;
  static UwbDiagnosticState* uwbState_;

  bool ready_ = false;
  uint32_t sequence_ = 0;

  void handleReceive(const uint8_t* data, int length);
  void applyPacket(const EspNowRangePacket& packet);
};
