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
#include "Max30102Diagnostics.h"
#include "NodeConfig.h"
#include "TimeUtils.h"
#include "UwbDiagnostics.h"
#include "UwbEventQueue.h"
#include "UwbRfQuality.h"
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

#ifndef BPMWATCH_MAX_TASK
#define BPMWATCH_MAX_TASK false
#endif

#ifndef BPMWATCH_MAX_TASK_PRIORITY
#define BPMWATCH_MAX_TASK_PRIORITY 3
#endif

#ifndef BPMWATCH_MAX_TASK_CORE
#define BPMWATCH_MAX_TASK_CORE 0
#endif

#ifndef BPMWATCH_MAX_TASK_STACK
#define BPMWATCH_MAX_TASK_STACK 6144
#endif

#ifndef BPMWATCH_MAX_SAMPLE_INTERVAL_MS
#define BPMWATCH_MAX_SAMPLE_INTERVAL_MS 10
#endif

#ifndef BPMWATCH_I2C_CLOCK_HZ
#define BPMWATCH_I2C_CLOCK_HZ 400000
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

void updateUwbTaskDiagnostics(uint32_t nowMs, uint32_t& lastStackReportMs) {
  if (lastStackReportMs != 0 &&
      safeAgeMs(nowMs, lastStackReportMs) < 1000) {
    return;
  }
  lastStackReportMs = nowMs;
  DiagnosticsLock lock;
  state.uwb.uwbTaskStackHighWater = uxTaskGetStackHighWaterMark(nullptr);
  state.uwb.uwbInterruptCount = uwbInterruptWakeCount;
  state.uwb.uwbIrqPinLevel = digitalRead(kUwbIrqPin) ? 1 : 0;
}

void uwbTask(void*) {
  uint32_t lastStackReportMs = 0;
  for (;;) {
    updateUwbTaskDiagnostics(millis(), lastStackReportMs);
    const uint32_t notificationCount =
        ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(2));
    const uint16_t pollCount = uwbPollsForTaskWake(
        notificationCount, BPMWATCH_UWB_IDLE_POLLS,
        BPMWATCH_UWB_IRQ_BURST_POLLS, BPMWATCH_UWB_POLLS_PER_TICK);
    if (pollCount == 0) {
      vTaskDelay(pdMS_TO_TICKS(2));
      continue;
    }
    for (uint16_t i = 0; i < pollCount; ++i) {
      uwb.poll(millis(), state.uwb);
    }
    updateUwbTaskDiagnostics(millis(), lastStackReportMs);
    vTaskDelay(pdMS_TO_TICKS(1));
  }
}
#endif

#if BPMWATCH_MAX_TASK && defined(ARDUINO_ARCH_ESP32)
TaskHandle_t maxTaskHandle = nullptr;

void maxTask(void*) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(BPMWATCH_MAX_SAMPLE_INTERVAL_MS);
  for (;;) {
    state.max30102.maxTaskStackHighWater = uxTaskGetStackHighWaterMark(nullptr);
    if (state.max30102.initialized) {
      // Keep the I2C read out of DiagnosticsLock so logging cannot throttle SPS.
      max30102.sample(millis(), state.max30102);
    }
    vTaskDelayUntil(&lastWake, period);
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
  const char* uwbStatus = "IDLE";
  if (!current.uwb.spiReady) {
    uwbStatus = "NO-SPI";
  } else if (current.uwb.hasRange && !current.uwb.rangeStale) {
    uwbStatus = "RANGING";
  } else if (current.uwb.peerPresent) {
    uwbStatus = "PEER";
  } else {
    uwbStatus = "DISCOVERY";
  }

  const char* losStatus = "---";
  if (current.uwb.rxLosNlosHint == 1) {
    losStatus = "LOS";
  } else if (current.uwb.rxLosNlosHint == 2) {
    losStatus = "MAYBE";
  } else if (current.uwb.rxLosNlosHint == 3) {
    losStatus = "NLOS";
  }
  const char* rfStatus =
      current.uwb.rxLosNlosHint == 0
          ? "---"
          : uwbRfQualityLabel(current.uwb.rxFpPowerDbm,
                              current.uwb.rxLosNlosDelta);

  const char* gyStatus =
      current.gy511.initialized
          ? (current.gy511.readOk
                 ? "OK"
                 : "RDERR")
          : "OFF";
  const char* maxStatus = current.max30102.initialized ? "OK" : "OFF";
  const char* maxReason = max30102BpmZeroReasonLabel(max30102BpmZeroReason(
      current.max30102.initialized, current.max30102.fingerPresent,
      current.max30102.averageBpm, current.max30102.maxSps,
      current.max30102.maxIrAc1s, current.max30102.maxBeatDetectCount,
      current.max30102.rejectedBeatCount));
  const char* irqMode =
      current.uwb.uwbInterruptCount > 0 ? "IRQ" : "SAFETY_POLL";

  Serial.printf(
      "t=%lu %s | UWB=%s peer=%d R#=%lu REC#=%lu AGE=%lums ACT=%lums RCAGE=%lums D=%.2fm STK=%lu IRQ#=%lu IRQPIN=%u IRQMODE=%s P#=%lu EV=%lu/%lu/%lu RST#=%lu RXERR=%lu/%lu RRST#=%lu REG=S:%02X%08lX M:%08lX C:%08lX CFG:%08lX DM:%u LR_OK=%d LR_FAIL=%u LR_FAIL_REASON=%s RX_POWER=%.1f FP_POWER=%.1f DELTA=%.1f LOS=%s RF=%s RXPACC=%u CIR_PWR=%u FP_AMPL=%u/%u/%u STD_NOISE=%u | ESPNOW=%s TX=%lu/%lu RX=%lu | "
      "GY=%s H=%.1f A=%d,%d,%d | MAX=%s IR=%ld BPM=%d SPS=%lu BEAT#=%lu IR_MIN=%ld IR_MAX=%ld IRAC=%ld LASTBEAT=%lums REJ#=%lu BI=%lums WHY=%s MAX_DUR_US=%lu MAX_DUR_MAX_US=%lu MAX_LOCK_FAIL=%lu MAX_TASK_STK=%lu\n",
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
      irqMode,
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
      current.uwb.longRangeConfigOk ? 1 : 0,
      static_cast<unsigned>(current.uwb.regConfigFailures),
      current.uwb.regConfigFailureReason,
      current.uwb.rxPowerDbm,
      current.uwb.rxFpPowerDbm,
      current.uwb.rxLosNlosDelta,
      losStatus,
      rfStatus,
      static_cast<unsigned>(current.uwb.rxPacc),
      static_cast<unsigned>(current.uwb.rxCirPower),
      static_cast<unsigned>(current.uwb.rxFpAmpl1),
      static_cast<unsigned>(current.uwb.rxFpAmpl2),
      static_cast<unsigned>(current.uwb.rxFpAmpl3),
      static_cast<unsigned>(current.uwb.rxStdNoise),
      current.uwb.espNowReady ? "OK" : "OFF",
      static_cast<unsigned long>(current.uwb.espNowTxCount),
      static_cast<unsigned long>(current.uwb.espNowTxFailCount),
      static_cast<unsigned long>(current.uwb.espNowRxCount), gyStatus,
      current.gy511.headingDeg,
      current.gy511.accelX, current.gy511.accelY, current.gy511.accelZ,
      maxStatus, current.max30102.irValue, current.max30102.averageBpm,
      static_cast<unsigned long>(current.max30102.maxSps),
      static_cast<unsigned long>(current.max30102.maxBeatDetectCount),
      static_cast<long>(current.max30102.maxIrMin1s),
      static_cast<long>(current.max30102.maxIrMax1s),
      static_cast<long>(current.max30102.maxIrAc1s),
      static_cast<unsigned long>(current.max30102.lastBeatAgeMs),
      static_cast<unsigned long>(current.max30102.rejectedBeatCount),
      static_cast<unsigned long>(current.max30102.lastBeatIntervalMs),
      maxReason,
      static_cast<unsigned long>(current.max30102.maxSampleDurationUs),
      static_cast<unsigned long>(current.max30102.maxSampleDurationMaxUs),
      static_cast<unsigned long>(current.max30102.maxLockFailCount),
      static_cast<unsigned long>(current.max30102.maxTaskStackHighWater));
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
  Wire.setClock(BPMWATCH_I2C_CLOCK_HZ);
  Serial.printf("I2C clock: %lu Hz\n",
                static_cast<unsigned long>(BPMWATCH_I2C_CLOCK_HZ));
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
  pinMode(kUwbIrqPin, INPUT);
  const BaseType_t uwbTaskCreateResult = xTaskCreatePinnedToCore(
      uwbTask, "UWBTask", BPMWATCH_UWB_TASK_STACK, nullptr,
      BPMWATCH_UWB_TASK_PRIORITY, &uwbTaskHandle, BPMWATCH_UWB_TASK_CORE);
  const bool uwbTaskCreated = uwbTaskCreateResult == pdPASS;
  const int uwbIrqNumber = digitalPinToInterrupt(kUwbIrqPin);
  const bool uwbGpioIrqAttached = uwbTaskCreated && uwbIrqNumber >= 0;
  DW1000.attachInterruptCompleteHandler(wakeUwbTaskFromDw1000Interrupt);
  if (uwbGpioIrqAttached) {
    attachInterrupt(uwbIrqNumber, wakeUwbTaskFromDw1000Interrupt, RISING);
  }
  Serial.printf(
      "UWB task: enabled=1 created=%d priority=%d core=%d stack=%d idle_polls=%d irq_burst=%d polls_per_tick=%d gpio_irq=%s irq_pin=%d\n",
      uwbTaskCreated ? 1 : 0, BPMWATCH_UWB_TASK_PRIORITY,
      BPMWATCH_UWB_TASK_CORE, BPMWATCH_UWB_TASK_STACK,
      BPMWATCH_UWB_IDLE_POLLS, BPMWATCH_UWB_IRQ_BURST_POLLS,
      BPMWATCH_UWB_POLLS_PER_TICK, uwbGpioIrqAttached ? "ATTACHED" : "OFF",
      kUwbIrqPin);
  if (!uwbTaskCreated) {
    Serial.println("UWB task warning: task create failed; loop fallback is disabled for this build");
  }
#else
  Serial.println("UWB task: enabled=0");
#endif

#if BPMWATCH_MAX_TASK && defined(ARDUINO_ARCH_ESP32)
  const BaseType_t maxTaskCreateResult = xTaskCreatePinnedToCore(
      maxTask, "MAXTask", BPMWATCH_MAX_TASK_STACK, nullptr,
      BPMWATCH_MAX_TASK_PRIORITY, &maxTaskHandle, BPMWATCH_MAX_TASK_CORE);
  state.max30102.maxTaskCreated = maxTaskCreateResult == pdPASS;
  Serial.printf(
      "MAX task: enabled=1 created=%d priority=%d core=%d stack=%d interval=%dms\n",
      state.max30102.maxTaskCreated ? 1 : 0, BPMWATCH_MAX_TASK_PRIORITY,
      BPMWATCH_MAX_TASK_CORE, BPMWATCH_MAX_TASK_STACK,
      BPMWATCH_MAX_SAMPLE_INTERVAL_MS);
  if (!state.max30102.maxTaskCreated) {
    Serial.println("MAX task warning: task create failed; loop fallback is disabled for this build");
  }
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

#if !BPMWATCH_MAX_TASK
  if (nowMs - lastMaxMs >= sensorIntervalMs) {
    lastMaxMs = nowMs;
    max30102.sample(nowMs, state.max30102);
  }
#endif
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
