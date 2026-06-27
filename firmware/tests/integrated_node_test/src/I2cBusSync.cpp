#include "I2cBusSync.h"

#if defined(ARDUINO_ARCH_ESP32)
SemaphoreHandle_t gI2cBusMutex = nullptr;

void beginI2cBusSync() {
  if (gI2cBusMutex == nullptr) {
    gI2cBusMutex = xSemaphoreCreateMutex();
  }
}

I2cBusLock::I2cBusLock(uint32_t timeoutMs) {
  if (gI2cBusMutex == nullptr) {
    beginI2cBusSync();
  }
  acquired_ =
      gI2cBusMutex != nullptr &&
      xSemaphoreTake(gI2cBusMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

I2cBusLock::~I2cBusLock() {
  if (acquired_) {
    xSemaphoreGive(gI2cBusMutex);
  }
}
#endif
