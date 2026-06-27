#pragma once

#include <stdint.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t gI2cBusMutex;

void beginI2cBusSync();

class I2cBusLock {
 public:
  explicit I2cBusLock(uint32_t timeoutMs = 20);
  ~I2cBusLock();
  bool acquired() const { return acquired_; }

 private:
  bool acquired_ = false;
};
#else
inline void beginI2cBusSync() {}

class I2cBusLock {
 public:
  explicit I2cBusLock(uint32_t = 20) {}
  bool acquired() const { return true; }
};
#endif
