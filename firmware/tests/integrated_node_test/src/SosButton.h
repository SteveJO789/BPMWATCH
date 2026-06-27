#pragma once

#include <stdint.h>

#include "TimeUtils.h"

enum class SosButtonEvent : uint8_t {
  None,
  Pressed,
  ShortPress,
  LongPressToggle,
};

struct SosButtonConfig {
  uint32_t debounceMs = 50;
  uint32_t longPressMs = 1000;
};

struct SosButtonInput {
  uint32_t nowMs = 0;
  bool pressed = false;
};

struct SosButtonState {
  bool sosActive = false;
  uint32_t sosSeq = 0;
  bool rawPressed = false;
  bool stablePressed = false;
  bool longPressHandled = false;
  uint32_t rawChangedMs = 0;
  uint32_t stableChangedMs = 0;
  uint32_t pressStartMs = 0;
  uint32_t lastEventDurationMs = 0;
  SosButtonEvent lastEvent = SosButtonEvent::None;
};

struct RemoteSosState {
  bool active = false;
  uint32_t seq = 0;
  uint32_t lastRxMs = 0;
  uint8_t senderNodeId = 0;
};

constexpr uint32_t kRemoteSosTimeoutMs = 10000;

inline const char* sosButtonEventLabel(SosButtonEvent event) {
  switch (event) {
    case SosButtonEvent::None:
      return "NONE";
    case SosButtonEvent::Pressed:
      return "PRESS";
    case SosButtonEvent::ShortPress:
      return "SHORT";
    case SosButtonEvent::LongPressToggle:
      return "LONG";
  }
  return "UNKNOWN";
}

inline void updateSosButton(const SosButtonInput& input,
                            SosButtonState& state,
                            const SosButtonConfig& config) {
  state.lastEvent = SosButtonEvent::None;
  if (input.pressed != state.rawPressed) {
    state.rawPressed = input.pressed;
    state.rawChangedMs = input.nowMs;
  }

  if (state.rawPressed != state.stablePressed &&
      safeAgeMs(input.nowMs, state.rawChangedMs) >= config.debounceMs) {
    state.stablePressed = state.rawPressed;
    state.stableChangedMs = input.nowMs;
    if (state.stablePressed) {
      state.pressStartMs = input.nowMs;
      state.longPressHandled = false;
      state.lastEventDurationMs = 0;
      state.lastEvent = SosButtonEvent::Pressed;
    } else {
      const uint32_t pressDuration =
          safeAgeMs(input.nowMs, state.pressStartMs);
      state.lastEventDurationMs = pressDuration;
      if (!state.longPressHandled && pressDuration < config.longPressMs) {
        state.lastEvent = SosButtonEvent::ShortPress;
      }
    }
  }

  if (state.stablePressed && !state.longPressHandled &&
      safeAgeMs(input.nowMs, state.pressStartMs) >= config.longPressMs) {
    state.sosActive = !state.sosActive;
    ++state.sosSeq;
    state.longPressHandled = true;
    state.lastEventDurationMs = safeAgeMs(input.nowMs, state.pressStartMs);
    state.lastEvent = SosButtonEvent::LongPressToggle;
  }
}

inline bool remoteSosVisible(const RemoteSosState& remote, uint32_t nowMs) {
  return remote.active &&
         safeAgeMs(nowMs, remote.lastRxMs) <= kRemoteSosTimeoutMs;
}
