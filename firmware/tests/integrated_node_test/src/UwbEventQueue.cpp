#include "UwbEventQueue.h"

#include <Arduino.h>

#if defined(ARDUINO_ARCH_ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#endif

namespace {
#if defined(ARDUINO_ARCH_ESP32)
QueueHandle_t gUwbEventQueue = nullptr;
#endif
}

void beginUwbEventQueue() {
#if defined(ARDUINO_ARCH_ESP32)
  if (gUwbEventQueue == nullptr) {
    gUwbEventQueue = xQueueCreate(16, sizeof(UwbEvent));
  }
#endif
}

bool pushUwbEvent(const UwbEvent& event) {
#if defined(ARDUINO_ARCH_ESP32)
  if (gUwbEventQueue == nullptr) {
    return false;
  }
  return xQueueSendToBack(gUwbEventQueue, &event, 0) == pdTRUE;
#else
  (void)event;
  return false;
#endif
}

bool popUwbEvent(UwbEvent& event) {
#if defined(ARDUINO_ARCH_ESP32)
  if (gUwbEventQueue == nullptr) {
    return false;
  }
  return xQueueReceive(gUwbEventQueue, &event, 0) == pdTRUE;
#else
  (void)event;
  return false;
#endif
}

void printUwbEvent(const UwbEvent& event) {
  switch (event.type) {
    case UwbEventType::DeviceId:
      Serial.printf("DW1000 Device ID: %s\n", event.text);
      break;
    case UwbEventType::Recovery:
      Serial.printf("UWB recovery #%lu after %lu ms\n",
                    static_cast<unsigned long>(event.valueA),
                    static_cast<unsigned long>(event.valueB));
      break;
    case UwbEventType::PeerAdded:
      Serial.printf("UWB peer added: 0x%04lX\n",
                    static_cast<unsigned long>(event.valueA));
      break;
    case UwbEventType::PeerInactive:
      Serial.printf("UWB peer inactive: 0x%04lX\n",
                    static_cast<unsigned long>(event.valueA));
      break;
  }
}
