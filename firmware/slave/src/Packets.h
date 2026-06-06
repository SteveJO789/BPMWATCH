#pragma once

#include <Arduino.h>

constexpr uint8_t NODE_MASTER = 0;
constexpr uint8_t NODE_SLAVE_1 = 1;
constexpr uint8_t NODE_SLAVE_2 = 2;

typedef struct {
  uint8_t id;
  int bpm;
  int battery;
  uint8_t signal;
} SlaveStatusPacket;

typedef struct {
  uint8_t fromId;
  uint8_t toId;
  float distanceMeters;
  uint32_t timestampMs;
  uint8_t quality;
} DistancePacket;

typedef struct {
  float masterX;
  float masterY;
  float slave1X;
  float slave1Y;
  float slave2X;
  float slave2Y;
  int masterBattery;
  int slave1Bpm;
  int slave1Battery;
  int slave2Bpm;
  int slave2Battery;
  uint8_t mapValid;
} TeamMapPacket;
