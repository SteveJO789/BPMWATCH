#include <Arduino.h>

#include "FeatureFlags.h"
#if BPMWATCH_NEEDS_I2C
#include <Wire.h>
#endif
#if BPMWATCH_ENABLE_DISPLAY
#include "DiagnosticsDisplay.h"
#endif
#include "DiagnosticsState.h"
#include "DiagnosticsSync.h"
#if BPMWATCH_ESPNOW_RANGE_LINK
#include "EspNowRangeLink.h"
#endif
#if BPMWATCH_ENABLE_GY511
#include "Gy511Sensor.h"
#endif
#if BPMWATCH_ENABLE_I2C_SCAN
#include "I2cScan.h"
#endif
#if BPMWATCH_ENABLE_MAX30102
#include "Max30102Sensor.h"
#endif
#include "NodeConfig.h"
#include "UwbDiagnostics.h"
#include "UwbEventQueue.h"
#include "UwbTaskCadence.h"
#if BPMWATCH_UWB_TASK && defined(ARDUINO_ARCH_ESP32)
#include <DW1000.h>
#endif

#ifndef BPMWATCH_UWB_TASK
#define BPMWATCH_UWB_TASK false
#endif

#ifndef BPMWATCH_UWB_TASK_PRIORITY
#define BPMWATCH_UWB_TASK_PRIORITY 3
#endif

#ifndef BPMWATCH_UWB_TASK_CORE
#define BPMWATCH_UWB_TASK_CORE 1
#endif

#ifndef BPMWATCH_UWB_TASK_STACK
#define BPMWATCH_UWB_TASK_STACK 8192
#endif

#ifndef BPMWATCH_UWB_POLLS_PER_TICK
#define BPMWATCH_UWB_POLLS_PER_TICK 64
#endif

#ifndef BPMWATCH_UWB_IDLE_POLLS
#define BPMWATCH_UWB_IDLE_POLLS 1
#endif

#ifndef BPMWATCH_UWB_IRQ_BURST_POLLS
#define BPMWATCH_UWB_IRQ_BURST_POLLS 16
#endif

#ifndef BPMWATCH_DISCOVERY_DISPLAY_INTERVAL_MS
#define BPMWATCH_DISCOVERY_DISPLAY_INTERVAL_MS 1000
#endif

#ifndef BPMWATCH_CONNECTED_DISPLAY_INTERVAL_MS
#define BPMWATCH_CONNECTED_DISPLAY_INTERVAL_MS 300
#endif

#ifndef BPMWATCH_DISCOVERY_SENSOR_INTERVAL_MS
#define BPMWATCH_DISCOVERY_SENSOR_INTERVAL_MS 250
#endif

#ifndef BPMWATCH_CONNECTED_SENSOR_INTERVAL_MS
#define BPMWATCH_CONNECTED_SENSOR_INTERVAL_MS 50
#endif

#ifndef BPMWATCH_MAX_REINIT_INTERVAL_MS
#define BPMWATCH_MAX_REINIT_INTERVAL_MS 2000
#endif

#ifndef BPMWATCH_DISPLAY_FULL_REFRESH_MS
#define BPMWATCH_DISPLAY_FULL_REFRESH_MS 5000
#endif

namespace {
constexpr int kUwbIrqPin = 34;

DiagnosticsState state;
#if BPMWATCH_ENABLE_DISPLAY
DiagnosticsDisplay display;
uint32_t lastDisplayMs = 0;
uint32_t lastDisplayFullRefreshMs = 0;
#endif
#if BPMWATCH_ENABLE_GY511
Gy511Sensor gy511;
#endif
#if BPMWATCH_ENABLE_MAX30102
Max30102Sensor max30102;
#endif
UwbDiagnostics uwb;
#if BPMWATCH_ESPNOW_RANGE_LINK
EspNowRangeLink espNowRange;
#endif

#if BPMWATCH_ENABLE_MAX30102
uint32_t lastMaxMs = 0;
uint32_t lastMaxInitAttemptMs = 0;
uint32_t maxInitAttemptCount = 0;
#endif
#if BPMWATCH_ENABLE_GY511
uint32_t lastGyMs = 0;
#endif
#if BPMWATCH_ESPNOW_RANGE_LINK
uint32_t lastEspNowSentRangeCount = 0;
#endif
uint32_t lastLogMs = 0;

#if BPMWATCH_UWB_TASK && defined(ARDUINO_ARCH_ESP32)
TaskHandle_t uwbTaskHandle = nullptr;
volatile uint32_t uwbInterruptWakeCount = 0;

void IRAM_ATTR wakeUwbTaskFromDw1000Interrupt() {
  ++uwbInterruptWakeCount;
  BaseType_t higherPriorityTaskWoken = pdFALSE;
  if (uwbTaskHandle != nullptr) {
    vTaskNotifyGiveFromISR(uwbTaskHandle, &higherPriorityTaskWoken);
  }
  if (higherPriorityTaskWoken == pdTRUE) {
    portYIELD_FROM_ISR();
  }
}

void uwbTask(void*) {
  uint32_t lastStackReportMs = 0;
  for (;;) {
    const uint32_t notificationCount =
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(1));
    const uint16_t pollCount = uwbPollsForTaskWake(
        notificationCount, BPMWATCH_UWB_IDLE_POLLS,
        BPMWATCH_UWB_IRQ_BURST_POLLS);
    for (uint16_t i = 0; i < pollCount; ++i) {
      uwb.poll(millis(), state.uwb);
    }
#if defined(ARDUINO_ARCH_ESP32)
    const uint32_t nowMs = millis();
    if (nowMs - lastStackReportMs >= 1000) {
      lastStackReportMs = nowMs;
      DiagnosticsLock lock;
      state.uwb.uwbTaskStackHighWater = uxTaskGetStackHighWaterMark(nullptr);
      state.uwb.uwbInterruptCount = uwbInterruptWakeCount;
      state.uwb.uwbIrqPinLevel = digitalRead(kUwbIrqPin) ? 1 : 0;
    }
#endif
  }
}
#endif

UwbDiagnosticState copyUwbState() {
  DiagnosticsLock lock;
  return state.uwb;
}

DiagnosticsState copyDiagnosticsState() {
  DiagnosticsLock lock;
  return state;
}

void drainUwbEvents() {
  UwbEvent event;
  while (popUwbEvent(event)) {
    printUwbEvent(event);
  }
}

#if BPMWATCH_ENABLE_I2C_SCAN
void scanI2cBus(TwoWire& wire, Gy511DiagnosticState& gyState) {
  uint8_t addresses[16]{};
  uint8_t count = 0;
  for (uint8_t address = 1; address < 127 && count < sizeof(addresses); ++address) {
    wire.beginTransmission(address);
    if (wire.endTransmission() == 0) {
      addresses[count++] = address;
    }
  }
  formatI2cAddressList(addresses, count, gyState.i2cAddresses,
                       sizeof(gyState.i2cAddresses));
}
#endif

#if BPMWATCH_ENABLE_MAX30102
void tryBeginMax30102(TwoWire& wire, bool logResult) {
  ++maxInitAttemptCount;
  state.max30102.initialized = max30102.begin(wire);
  if (state.max30102.initialized) {
    state.max30102.irValue = 0;
    state.max30102.fingerPresent = false;
    state.max30102.bpm = 0.0f;
    state.max30102.averageBpm = 0;
  }
  if (logResult) {
    Serial.printf("MAX30102 init attempt #%lu: %s\n",
                  static_cast<unsigned long>(maxInitAttemptCount),
                  state.max30102.initialized ? "OK" : "ERROR");
  }
}
#endif

void logDiagnostics(const DiagnosticsState& current, uint32_t nowMs) {
  const char* uwbStatus = !current.uwb.spiReady
                              ? "SPI_ERR"
                          : current.uwb.rangeStale ? "LOST"
                          : current.uwb.hasRange   ? "OK"
                                                   : "WAIT";
  const char* gyStatus = current.gy511.initialized && current.gy511.readOk
                             ? "OK"
                             : gy511StatusLabel(current.gy511.status);
  const char* maxStatus = !current.max30102.initialized
                              ? "ERR"
                          : current.max30102.fingerPresent ? "OK"
                                                           : "NO_FINGER";

  Serial.printf(
      "t=%lu %s | UWB=%s peer=%d R#=%lu REC#=%lu AGE=%lums ACT=%lums RCAGE=%lums D=%.2fm STK=%lu IRQ#=%lu IRQPIN=%u P#=%lu EV=%lu/%lu/%lu RST#=%lu RXERR=%lu/%lu RRST#=%lu REG=S:%02X%08lX M:%08lX C:%08lX CFG:%08lX DM:%u | ESPNOW=%s TX=%lu/%lu RX=%lu | "
      "GY=%s H=%.1f A=%d,%d,%d | MAX=%s IR=%ld BPM=%d\n",
      static_cast<unsigned long>(nowMs), kNodeConfig.displayLabel, uwbStatus,
      current.uwb.peerPresent ? 1 : 0,
      static_cast<unsigned long>(current.uwb.rangeCount),
      static_cast<unsigned long>(current.uwb.recoveryCount),
      static_cast<unsigned long>(current.uwb.rangeAgeMs),
      static_cast<unsigned long>(current.uwb.uwbActivityAgeMs),
      static_cast<unsigned long>(current.uwb.uwbRecoveryAgeMs),
      current.uwb.distanceM,
      static_cast<unsigned long>(current.uwb.uwbTaskStackHighWater),
      static_cast<unsigned long>(current.uwb.uwbInterruptCount),
      static_cast<unsigned>(current.uwb.uwbIrqPinLevel),
      static_cast<unsigned long>(current.uwb.uwbPollCount),
      static_cast<unsigned long>(current.uwb.uwbRangeEventCount),
      static_cast<unsigned long>(current.uwb.uwbPeerEventCount),
      static_cast<unsigned long>(current.uwb.uwbInactiveEventCount),
      static_cast<unsigned long>(current.uwb.uwbRestartCount),
      static_cast<unsigned long>(current.uwb.uwbRxFailureCount),
      static_cast<unsigned long>(current.uwb.uwbRxTimeoutCount),
      static_cast<unsigned long>(current.uwb.uwbReceiverResetCount),
      static_cast<unsigned>(current.uwb.uwbSysStatusHigh),
      static_cast<unsigned long>(current.uwb.uwbSysStatusLow),
      static_cast<unsigned long>(current.uwb.uwbSysMask),
      static_cast<unsigned long>(current.uwb.uwbSysCtrl),
      static_cast<unsigned long>(current.uwb.uwbSysCfg),
      static_cast<unsigned>(current.uwb.uwbDeviceMode),
      current.uwb.espNowReady ? "OK" : "OFF",
      static_cast<unsigned long>(current.uwb.espNowTxCount),
      static_cast<unsigned long>(current.uwb.espNowTxFailCount),
      static_cast<unsigned long>(current.uwb.espNowRxCount), gyStatus,
      current.gy511.headingDeg,
      current.gy511.accelX, current.gy511.accelY, current.gy511.accelZ,
      maxStatus, current.max30102.irValue, current.max30102.averageBpm);
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.printf("BPMWATCH integrated diagnostics: %s\n",
                kNodeConfig.displayLabel);
  beginDiagnosticsSync();
  beginUwbEventQueue();

#if BPMWATCH_ENABLE_I2C_SCAN || BPMWATCH_ENABLE_GY511 || BPMWATCH_ENABLE_MAX30102
  Wire.begin(21, 22);
#endif

#if BPMWATCH_ENABLE_DISPLAY
  display.begin(kNodeConfig.displayLabel);
#else
  Serial.println("DISPLAY: OFF");
#endif

#if BPMWATCH_ENABLE_I2C_SCAN
  scanI2cBus(Wire, state.gy511);
  Serial.printf("I2C scan: %s\n", state.gy511.i2cAddresses);
#else
  Serial.println("I2C scan: OFF");
#endif

#if BPMWATCH_ENABLE_GY511
  gy511.begin(Wire, state.gy511);
  Serial.printf("GY-511 init: %s\n",
                state.gy511.initialized ? "OK" : "ERROR");
#else
  Serial.println("GY-511: OFF");
#endif

#if BPMWATCH_ENABLE_MAX30102
  tryBeginMax30102(Wire, true);
#else
  Serial.println("MAX30102: OFF");
#endif

#if BPMWATCH_ESPNOW_RANGE_LINK
  Serial.printf("ESP-NOW range link: %s\n",
                espNowRange.begin(state.uwb) ? "OK" : "ERROR");
#else
  Serial.println("ESP-NOW range link: OFF");
#endif

  uwb.begin(state.uwb);

#if BPMWATCH_UWB_TASK && defined(ARDUINO_ARCH_ESP32)
  xTaskCreatePinnedToCore(uwbTask, "UWBTask", BPMWATCH_UWB_TASK_STACK, nullptr,
                          BPMWATCH_UWB_TASK_PRIORITY, &uwbTaskHandle,
                          BPMWATCH_UWB_TASK_CORE);
  DW1000.attachInterruptCompleteHandler(wakeUwbTaskFromDw1000Interrupt);
  Serial.printf(
      "UWB task: ON priority=%d core=%d stack=%d idle_polls=%d irq_burst=%d\n",
                BPMWATCH_UWB_TASK_PRIORITY, BPMWATCH_UWB_TASK_CORE,
                BPMWATCH_UWB_TASK_STACK, BPMWATCH_UWB_IDLE_POLLS,
                BPMWATCH_UWB_IRQ_BURST_POLLS);
#else
  Serial.println("UWB task: OFF");
#endif
}

void loop() {
  const uint32_t nowMs = millis();
#if !BPMWATCH_UWB_TASK
  uwb.poll(nowMs, state.uwb);
#endif
  drainUwbEvents();
  const UwbDiagnosticState uwbSnapshot = copyUwbState();

#if BPMWATCH_ESPNOW_RANGE_LINK
  if (kNodeConfig.isAnchor && uwbSnapshot.hasRange &&
      uwbSnapshot.rangeCount != lastEspNowSentRangeCount) {
    lastEspNowSentRangeCount = uwbSnapshot.rangeCount;
    espNowRange.sendRange(uwbSnapshot, nowMs);
  }
#endif

  updateRadarState(
      {uwbSnapshot.distanceM, uwbSnapshot.hasRange && !uwbSnapshot.rangeStale,
       nowMs},
      state.radar);

#if BPMWATCH_ENABLE_GY511 || BPMWATCH_ENABLE_MAX30102
  const uint32_t sensorIntervalMs =
      uwbSnapshot.hasRange ? BPMWATCH_CONNECTED_SENSOR_INTERVAL_MS
                           : BPMWATCH_DISCOVERY_SENSOR_INTERVAL_MS;
#endif

#if BPMWATCH_ENABLE_MAX30102
  if (!state.max30102.initialized &&
      nowMs - lastMaxInitAttemptMs >= BPMWATCH_MAX_REINIT_INTERVAL_MS) {
    lastMaxInitAttemptMs = nowMs;
    tryBeginMax30102(Wire, true);
  }

  if (nowMs - lastMaxMs >= sensorIntervalMs) {
    lastMaxMs = nowMs;
    max30102.sample(nowMs, state.max30102);
  }
#endif

#if BPMWATCH_ENABLE_GY511
  if (nowMs - lastGyMs >= sensorIntervalMs) {
    lastGyMs = nowMs;
    gy511.sample(state.gy511);
  }
#endif

#if BPMWATCH_ENABLE_DISPLAY
  const uint32_t displayIntervalMs =
      uwbSnapshot.hasRange ? BPMWATCH_CONNECTED_DISPLAY_INTERVAL_MS
                           : BPMWATCH_DISCOVERY_DISPLAY_INTERVAL_MS;
  if (nowMs - lastDisplayMs >= displayIntervalMs) {
    lastDisplayMs = nowMs;
    const bool forceFullDisplayRefresh =
        nowMs - lastDisplayFullRefreshMs >= BPMWATCH_DISPLAY_FULL_REFRESH_MS;
    if (forceFullDisplayRefresh) {
      lastDisplayFullRefreshMs = nowMs;
    }
    display.render(copyDiagnosticsState(), nowMs, forceFullDisplayRefresh);
  }
#endif

  if (nowMs - lastLogMs >= 1000) {
    lastLogMs = nowMs;
    logDiagnostics(copyDiagnosticsState(), nowMs);
  }
}
