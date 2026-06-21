#pragma once

#include <Arduino.h>

#ifndef BPMWATCH_NODE_ID
#error "BPMWATCH_NODE_ID must be 0 (Node A) or 1 (Node B)"
#endif

struct NodeConfig {
  uint8_t nodeId;
  bool isAnchor;
  const char* displayLabel;
  const char* uwbEui;
};

#if BPMWATCH_NODE_ID == 0
inline constexpr NodeConfig kNodeConfig{
    0, true, "NODE A / ANCHOR", "0C:8A:D3:7C:E5:A4:00:01"};
#elif BPMWATCH_NODE_ID == 1
inline constexpr NodeConfig kNodeConfig{
    1, false, "NODE B / TAG", "1C:75:C4:F4:E9:D4:00:01"};
#else
#error "Unsupported BPMWATCH_NODE_ID"
#endif

