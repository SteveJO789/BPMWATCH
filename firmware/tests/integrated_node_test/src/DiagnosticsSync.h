#pragma once

#if defined(ARDUINO_ARCH_ESP32)
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

extern SemaphoreHandle_t gDiagnosticsMutex;

void beginDiagnosticsSync();

class DiagnosticsLock {
 public:
  DiagnosticsLock();
  ~DiagnosticsLock();

 private:
  bool acquired_ = false;

  DiagnosticsLock(const DiagnosticsLock&) = delete;
  DiagnosticsLock& operator=(const DiagnosticsLock&) = delete;
};
#else
inline void beginDiagnosticsSync() {}

class DiagnosticsLock {
 public:
  DiagnosticsLock() = default;
  ~DiagnosticsLock() = default;
};
#endif
