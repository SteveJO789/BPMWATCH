#pragma once

#include <stdint.h>

enum class UwbEventType : uint8_t {
  DeviceId,
  Recovery,
  PeerAdded,
  PeerInactive,
};

struct UwbEvent {
  UwbEventType type = UwbEventType::DeviceId;
  uint32_t valueA = 0;
  uint32_t valueB = 0;
  char text[64]{};
};

void beginUwbEventQueue();
bool pushUwbEvent(const UwbEvent& event);
bool popUwbEvent(UwbEvent& event);
void printUwbEvent(const UwbEvent& event);
