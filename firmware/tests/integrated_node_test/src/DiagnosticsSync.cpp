#include "DiagnosticsSync.h"

#if defined(ARDUINO_ARCH_ESP32)
SemaphoreHandle_t gDiagnosticsMutex = nullptr;

void beginDiagnosticsSync() {
  if (gDiagnosticsMutex == nullptr) {
    gDiagnosticsMutex = xSemaphoreCreateMutex();
  }
}

DiagnosticsLock::DiagnosticsLock() {
  if (gDiagnosticsMutex == nullptr) {
    beginDiagnosticsSync();
  }
  acquired_ = gDiagnosticsMutex != nullptr &&
              xSemaphoreTake(gDiagnosticsMutex, portMAX_DELAY) == pdTRUE;
}

DiagnosticsLock::~DiagnosticsLock() {
  if (acquired_) {
    xSemaphoreGive(gDiagnosticsMutex);
  }
}
#endif
