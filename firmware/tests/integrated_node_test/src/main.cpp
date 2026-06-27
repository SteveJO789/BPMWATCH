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
#if BPMWATCH_ENABLE_COMPASS
#include "CompassSensor.h"
#endif
#if BPMWATCH_ESPNOW_RANGE_LINK
#include "EspNowRangeLink.h"
#endif
#if BPMWATCH_ENABLE_I2C_SCAN
#include "I2cScan.h"
#endif
#if BPMWATCH_NEEDS_I2C
#include "I2cBusSync.h"
#endif
#if BPMWATCH_ENABLE_MAX30102
#include "Max30102Sensor.h"
#endif
#include "Max30102Diagnostics.h"
#include "NodeConfig.h"
#include "SosButton.h"
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

#ifndef BPMWATCH_COMPASS_TASK
#define BPMWATCH_COMPASS_TASK true
#endif

#ifndef BPMWATCH_COMPASS_TASK_PRIORITY
#define BPMWATCH_COMPASS_TASK_PRIORITY 2
#endif

#ifndef BPMWATCH_COMPASS_TASK_CORE
#define BPMWATCH_COMPASS_TASK_CORE 0
#endif

#ifndef BPMWATCH_COMPASS_TASK_STACK
#define BPMWATCH_COMPASS_TASK_STACK 4096
#endif

#ifndef BPMWATCH_COMPASS_SAMPLE_INTERVAL_MS
#define BPMWATCH_COMPASS_SAMPLE_INTERVAL_MS 100
#endif

#ifndef BPMWATCH_COMPASS_CAL_LOG
#define BPMWATCH_COMPASS_CAL_LOG false
#endif

#ifndef BPMWATCH_COMPASS_CAL_LOG_INTERVAL_MS
#define BPMWATCH_COMPASS_CAL_LOG_INTERVAL_MS 2000
#endif

#ifndef BPMWATCH_I2C_CLOCK_HZ
#define BPMWATCH_I2C_CLOCK_HZ 400000
#endif

#ifndef BPMWATCH_I2C_LOCK_TIMEOUT_MS
#define BPMWATCH_I2C_LOCK_TIMEOUT_MS 20
#endif

#ifndef BPMWATCH_COMPASS_I2C_LOCK_TIMEOUT_MS
#define BPMWATCH_COMPASS_I2C_LOCK_TIMEOUT_MS 50
#endif

#ifndef BPMWATCH_SOS_BUTTON_PIN
#define BPMWATCH_SOS_BUTTON_PIN 32
#endif

#ifndef BPMWATCH_SOS_DEBOUNCE_MS
#define BPMWATCH_SOS_DEBOUNCE_MS 50
#endif

#ifndef BPMWATCH_SOS_LONG_PRESS_MS
#define BPMWATCH_SOS_LONG_PRESS_MS 1000
#endif

#ifndef BPMWATCH_SOS_POLL_INTERVAL_MS
#define BPMWATCH_SOS_POLL_INTERVAL_MS 20
#endif

#ifndef BPMWATCH_ESPNOW_TX_INTERVAL_MS
#define BPMWATCH_ESPNOW_TX_INTERVAL_MS 1000
#endif

#ifndef BPMWATCH_ESPNOW_TASK
#define BPMWATCH_ESPNOW_TASK true
#endif

#ifndef BPMWATCH_ESPNOW_TASK_PRIORITY
#define BPMWATCH_ESPNOW_TASK_PRIORITY 1
#endif

#ifndef BPMWATCH_ESPNOW_TASK_CORE
#define BPMWATCH_ESPNOW_TASK_CORE 0
#endif

#ifndef BPMWATCH_ESPNOW_TASK_STACK
#define BPMWATCH_ESPNOW_TASK_STACK 4096
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
#if BPMWATCH_ENABLE_COMPASS
CompassSensor compass;
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
#if BPMWATCH_ENABLE_COMPASS
uint32_t lastCompassMs = 0;
#if BPMWATCH_COMPASS_CAL_LOG
uint32_t lastCompassCalLogMs = 0;
#endif
#endif
#if BPMWATCH_ESPNOW_RANGE_LINK
uint32_t lastEspNowTxMs = 0;
#if BPMWATCH_ESPNOW_TASK && defined(ARDUINO_ARCH_ESP32)
TaskHandle_t espNowTaskHandle = nullptr;
bool espNowTaskCreated = false;
#endif
#endif
#if BPMWATCH_ENABLE_SOS
SosButtonConfig sosConfig{BPMWATCH_SOS_DEBOUNCE_MS,
                          BPMWATCH_SOS_LONG_PRESS_MS};
uint32_t lastSosPollMs = 0;
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
      I2cBusLock i2cLock(BPMWATCH_I2C_LOCK_TIMEOUT_MS);
      if (i2cLock.acquired()) {
        max30102.sample(millis(), state.max30102);
      } else {
        ++state.max30102.maxLockFailCount;
      }
    }
    vTaskDelayUntil(&lastWake, period);
  }
}
#endif

#if BPMWATCH_ENABLE_COMPASS
void sampleCompass(uint32_t nowMs) {
  if (!state.compass.initialized) {
    updateCompassAge(state.compass, nowMs);
    return;
  }
  I2cBusLock i2cLock(BPMWATCH_COMPASS_I2C_LOCK_TIMEOUT_MS);
  if (i2cLock.acquired()) {
    compass.sample(nowMs, state.compass);
  } else {
    ++state.compass.i2cLockFailCount;
    state.compass.readOk = false;
    state.compass.lastIssue = CompassIssue::LockTimeout;
    updateCompassAge(state.compass, nowMs);
  }
}
#endif

#if BPMWATCH_ENABLE_COMPASS && BPMWATCH_COMPASS_TASK && defined(ARDUINO_ARCH_ESP32)
TaskHandle_t compassTaskHandle = nullptr;

void compassTask(void*) {
  TickType_t lastWake = xTaskGetTickCount();
  const TickType_t period = pdMS_TO_TICKS(BPMWATCH_COMPASS_SAMPLE_INTERVAL_MS);
  for (;;) {
    state.compass.compassTaskStackHighWater =
        uxTaskGetStackHighWaterMark(nullptr);
    sampleCompass(millis());
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

#if BPMWATCH_ENABLE_MAX30102
bool max30102BpmTelemetryValid(const Max30102DiagnosticState& maxState) {
  return maxState.initialized && maxState.fingerPresent &&
         maxState.signalUsable && maxState.bpmValid &&
         maxState.averageBpm > 0;
}

void updateMax30102BpmLostAlert(uint32_t nowMs) {
  DiagnosticsLock lock;
  updateRadarBpmLostAlert(BPMWATCH_ENABLE_MAX30102,
                          max30102BpmTelemetryValid(state.max30102), nowMs,
                          state.max30102.bpmInvalidSinceMs,
                          state.max30102.bpmInvalidAgeMs,
                          state.max30102.bpmLostAlert);
}
#endif

#if BPMWATCH_ESPNOW_RANGE_LINK
void sendEspNowSnapshot(uint32_t nowMs) {
#if BPMWATCH_ENABLE_MAX30102
  updateMax30102BpmLostAlert(nowMs);
#endif
  espNowRange.sendTelemetry(copyDiagnosticsState(), nowMs);
}
#endif

#if BPMWATCH_ESPNOW_RANGE_LINK && BPMWATCH_ESPNOW_TASK && defined(ARDUINO_ARCH_ESP32)
void espNowTask(void*) {
  TickType_t lastWake = xTaskGetTickCount();
  for (;;) {
    sendEspNowSnapshot(millis());
    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(BPMWATCH_ESPNOW_TX_INTERVAL_MS));
  }
}
#endif

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
  formatI2cDeviceList(addresses, count, gyState.i2cDeviceList,
                      sizeof(gyState.i2cDeviceList));
}
#endif

#if BPMWATCH_ENABLE_MAX30102
void tryBeginMax30102(TwoWire& wire, bool logResult) {
  ++maxInitAttemptCount;
  {
    I2cBusLock i2cLock(BPMWATCH_I2C_LOCK_TIMEOUT_MS);
    if (i2cLock.acquired()) {
      state.max30102.initialized = max30102.begin(wire);
    } else {
      state.max30102.initialized = false;
      ++state.max30102.maxLockFailCount;
    }
  }
  state.max30102.irValue = 0;
  state.max30102.fingerPresent = false;
  state.max30102.bpm = 0.0f;
  state.max30102.averageBpm = 0;
  state.max30102.bpmValid = false;
  state.max30102.bpmInvalidSinceMs = 0;
  state.max30102.bpmInvalidAgeMs = 0;
  state.max30102.bpmLostAlert = false;
  state.max30102.signalUsable = false;
  state.max30102.stableBeatCount = 0;
  state.max30102.falseBeatGateCount = 0;
  state.max30102.noFingerResetCount = 0;
  state.max30102.lowSignalRejectCount = 0;
  state.max30102.refractoryBeatRejectCount = 0;
  state.max30102.intervalBeatRejectCount = 0;
  state.max30102.unstableBeatRejectCount = 0;
  state.max30102.irDcBaseline = 0;
  state.max30102.irAcGate = 0;
  state.max30102.maxBeatDetectCount = 0;
  state.max30102.acceptedBeatCount = 0;
  state.max30102.acceptedBeatSeeded = false;
  state.max30102.rawBeatIntervalMs = 0;
  state.max30102.candidateBeatIntervalMs = 0;
  state.max30102.acceptedBeatIntervalMs = 0;
  state.max30102.medianBeatIntervalMs = 0;
  state.max30102.lastBeatIntervalMs = 0;
  state.max30102.beatReseedCount = 0;
  state.max30102.irAcRatioPpm = 0;
  state.max30102.irAcThreshold = BPMWATCH_MAX30102_MIN_IR_AC;
  state.max30102.irAcRatioThresholdPpm =
      BPMWATCH_MAX30102_MIN_IR_AC_RATIO_PPM;
  state.max30102.wristSignalGate = false;
  state.max30102.wristIrAcThreshold = BPMWATCH_MAX30102_WRIST_IRAC_THR;
  state.max30102.wristIrAcExitThreshold =
      BPMWATCH_MAX30102_WRIST_IRAC_EXIT_THR;
  state.max30102.wristIrAcRatioThresholdPpm =
      BPMWATCH_MAX30102_WRIST_RATIO_THR_PPM;
  state.max30102.wristIrAcRatioExitPpm =
      BPMWATCH_MAX30102_WRIST_RATIO_EXIT_PPM;
  state.max30102.signalHold = false;
  state.max30102.signalLostMs = 0;
  state.max30102.wristEnterCount = 0;
  state.max30102.wristExitCount = 0;
  state.max30102.signalOkReason = Max30102SignalReasonOff;
  if (logResult) {
    Serial.printf("MAX30102 init attempt #%lu: %s\n",
                  static_cast<unsigned long>(maxInitAttemptCount),
                  state.max30102.initialized ? "OK" : "ERROR");
  }
}
#endif

#if BPMWATCH_ENABLE_SOS
void sampleSosButton(uint32_t nowMs) {
  const bool pressed = digitalRead(BPMWATCH_SOS_BUTTON_PIN) == LOW;
  updateSosButton({nowMs, pressed}, state.sos, sosConfig);
}
#endif

#if BPMWATCH_ENABLE_COMPASS && BPMWATCH_COMPASS_CAL_LOG
void logCompassCalibrationSnapshot(const CompassDiagnosticState& compassState) {
  Serial.printf(
      "COMPASS_CAL MAG_MIN=%d,%d,%d MAG_MAX=%d,%d,%d RAW=%d,%d,%d SAMPLES=%lu\n",
      compassState.magMinX, compassState.magMinY, compassState.magMinZ,
      compassState.magMaxX, compassState.magMaxY, compassState.magMaxZ,
      compassState.magX, compassState.magY, compassState.magZ,
      static_cast<unsigned long>(compassState.sampleCount));
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

  const char* compassStatus =
      current.compass.initialized ? compassHealthLabel(current.compass) : "OFF";
  const char* maxStatus = current.max30102.initialized ? "OK" : "OFF";
  const char* maxReason = max30102BpmZeroReasonLabel(max30102BpmZeroReason(
      current.max30102.initialized, current.max30102.fingerPresent,
      current.max30102.signalUsable, current.max30102.bpmValid,
      current.max30102.stableBeatCount, current.max30102.maxSps,
      current.max30102.maxIrAc1s, current.max30102.irAcRatioPpm,
      current.max30102.maxBeatDetectCount, current.max30102.rejectedBeatCount,
      current.max30102.refractoryBeatRejectCount,
      current.max30102.intervalBeatRejectCount,
      current.max30102.unstableBeatRejectCount));
  const char* maxSignalReason =
      max30102SignalOkReasonLabel(current.max30102.signalOkReason);
  const char* irqMode =
      current.uwb.uwbInterruptCount > 0 ? "IRQ" : "SAFETY_POLL";
  const bool remoteSosActive = remoteSosVisible(current.peer.remoteSos, nowMs);
  const char* radarMode =
      BPMWATCH_ENABLE_RADAR_HEADING ? "HEADING_UI" : "DEMO";

  Serial.printf(
      "t=%lu %s | UWB=%s peer=%d R#=%lu REC#=%lu AGE=%lums ACT=%lums RCAGE=%lums D=%.2fm STK=%lu IRQ#=%lu IRQPIN=%u IRQMODE=%s P#=%lu EV=%lu/%lu/%lu RST#=%lu RXERR=%lu/%lu RRST#=%lu REG=S:%02X%08lX M:%08lX C:%08lX CFG:%08lX DM:%u LR_OK=%d LR_FAIL=%u LR_FAIL_REASON=%s RX_POWER=%.1f FP_POWER=%.1f DELTA=%.1f LOS=%s RF=%s RXPACC=%u CIR_PWR=%u FP_AMPL=%u/%u/%u STD_NOISE=%u | ESPNOW=%s TX=%lu/%lu RX=%lu | "
      "COMPASS=%s CSTAT=%s C_ERR=%s MAG=%s MAG_ADDR=%s MAG_DRIVER=%s MAG_WHOAMI=0x%02X MAG_RAW=%d,%d,%d MAG_ABS=%lu MAG_DATA_OK=%d ACC=%s ACC_ADDR=%s ACC_DRIVER=%s ACC_RAW=%d,%d,%d ACC_ABS=%lu ACC_DATA_OK=%d COMPASS_MODE=%s HEADING_MODE=%s HDG=%.1f HDG_VALID=%d C_LOCK_FAIL=%lu C_READ_FAIL=%lu C_WRITE_FAIL=%lu C_LAST_FAIL_STAGE=%s CAL=%s CS#=%lu CAGE=%lums | SOS=%d SOS_SEQ=%lu BTN_RAW=%d BTN_EVENT=%s BTN_DUR_MS=%lu REMOTE_SOS=%d REMOTE_ID=%u REMOTE_HDG=%.1f RADAR_MODE=%s | "
      "MAX=%s IR=%ld FINGER=%d SIG_OK=%d BPM=%d BPM_VALID=%d BPM_BAD_AGE=%lums BPM_ALERT=%d SPS=%lu BEAT#=%lu RAWBEAT#=%lu ACCBEAT#=%lu SEED=%d IR_MIN=%ld IR_MAX=%ld IRAC=%ld IRAC_THR=%lu IRAC_RATIO=%luppm IRAC_RATIO_THR=%luppm WRIST_GATE=%d SIG_HOLD=%d SIG_LOST_MS=%lu WRIST_ENTER=%lu WRIST_EXIT=%lu SIG_OK_REASON=%s WRIST_IRAC_THR=%lu WRIST_IRAC_EXIT=%lu WRIST_RATIO_THR=%luppm WRIST_RATIO_EXIT=%luppm LASTBEAT=%lums REJ#=%lu BI=%lums RAW_BI=%lums CAND_BI=%lums ACC_BI=%lums MED_BI=%lums STABLE#=%u NOFINGER_RST#=%lu LOWSIG_REJ#=%lu REFRACTORY_REJ#=%lu INTERVAL_REJ#=%lu RESEED#=%lu UNSTABLE_REJ#=%lu FALSE_GATE#=%lu WRIST_MODE=%d BPM_SRC=MEDIAN_INTERVAL IR_THR=%ld BPM_RANGE=%d-%d WHY=%s MAX_DUR_US=%lu MAX_DUR_MAX_US=%lu MAX_LOCK_FAIL=%lu MAX_TASK_STK=%lu\n",
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
      static_cast<unsigned long>(current.uwb.espNowRxCount), compassStatus,
      compassCompactStatusLabel(current.compass),
      compassIssueLabel(current.compass),
      compassMagLabel(current.compass),
      compassMagAddressLabel(current.compass),
      compassMagDriverLabel(current.compass),
      static_cast<unsigned>(current.compass.magWhoAmI),
      current.compass.magX, current.compass.magY, current.compass.magZ,
      static_cast<unsigned long>(current.compass.magAbs),
      current.compass.magDataOk ? 1 : 0,
      compassAccelLabel(current.compass),
      compassAccelAddressLabel(current.compass),
      compassAccelDriverLabel(current.compass),
      current.compass.accelX, current.compass.accelY, current.compass.accelZ,
      static_cast<unsigned long>(current.compass.accAbs),
      current.compass.accelDataOk ? 1 : 0,
      compassModeLabel(current.compass),
      compassHeadingModeLabel(current.compass),
      current.compass.headingDeg,
      compassHeadingValid(current.compass) ? 1 : 0,
      static_cast<unsigned long>(current.compass.i2cLockFailCount),
      static_cast<unsigned long>(current.compass.i2cReadFailCount),
      static_cast<unsigned long>(current.compass.i2cWriteFailCount),
      compassFailStageLabel(current.compass),
      compassCalibrationStatusLabel(current.compass),
      static_cast<unsigned long>(current.compass.sampleCount),
      static_cast<unsigned long>(current.compass.lastUpdateAgeMs),
      current.sos.sosActive ? 1 : 0,
      static_cast<unsigned long>(current.sos.sosSeq),
      current.sos.rawPressed ? 1 : 0,
      sosButtonEventLabel(current.sos.lastEvent),
      static_cast<unsigned long>(current.sos.lastEventDurationMs),
      remoteSosActive ? 1 : 0,
      static_cast<unsigned>(current.peer.remoteSos.senderNodeId),
      current.peer.headingValid ? current.peer.headingDeg : -1.0f,
      radarMode,
      maxStatus, current.max30102.irValue,
      current.max30102.fingerPresent ? 1 : 0,
      current.max30102.signalUsable ? 1 : 0,
      current.max30102.averageBpm,
      current.max30102.bpmValid ? 1 : 0,
      static_cast<unsigned long>(current.max30102.bpmInvalidAgeMs),
      current.max30102.bpmLostAlert ? 1 : 0,
      static_cast<unsigned long>(current.max30102.maxSps),
      static_cast<unsigned long>(current.max30102.maxBeatDetectCount),
      static_cast<unsigned long>(current.max30102.maxBeatDetectCount),
      static_cast<unsigned long>(current.max30102.acceptedBeatCount),
      current.max30102.acceptedBeatSeeded ? 1 : 0,
      static_cast<long>(current.max30102.maxIrMin1s),
      static_cast<long>(current.max30102.maxIrMax1s),
      static_cast<long>(current.max30102.maxIrAc1s),
      static_cast<unsigned long>(current.max30102.irAcThreshold),
      static_cast<unsigned long>(current.max30102.irAcRatioPpm),
      static_cast<unsigned long>(current.max30102.irAcRatioThresholdPpm),
      current.max30102.wristSignalGate ? 1 : 0,
      current.max30102.signalHold ? 1 : 0,
      static_cast<unsigned long>(current.max30102.signalLostMs),
      static_cast<unsigned long>(current.max30102.wristEnterCount),
      static_cast<unsigned long>(current.max30102.wristExitCount),
      maxSignalReason,
      static_cast<unsigned long>(current.max30102.wristIrAcThreshold),
      static_cast<unsigned long>(current.max30102.wristIrAcExitThreshold),
      static_cast<unsigned long>(current.max30102.wristIrAcRatioThresholdPpm),
      static_cast<unsigned long>(current.max30102.wristIrAcRatioExitPpm),
      static_cast<unsigned long>(current.max30102.lastBeatAgeMs),
      static_cast<unsigned long>(current.max30102.rejectedBeatCount),
      static_cast<unsigned long>(current.max30102.lastBeatIntervalMs),
      static_cast<unsigned long>(current.max30102.rawBeatIntervalMs),
      static_cast<unsigned long>(current.max30102.candidateBeatIntervalMs),
      static_cast<unsigned long>(current.max30102.acceptedBeatIntervalMs),
      static_cast<unsigned long>(current.max30102.medianBeatIntervalMs),
      static_cast<unsigned>(current.max30102.stableBeatCount),
      static_cast<unsigned long>(current.max30102.noFingerResetCount),
      static_cast<unsigned long>(current.max30102.lowSignalRejectCount),
      static_cast<unsigned long>(current.max30102.refractoryBeatRejectCount),
      static_cast<unsigned long>(current.max30102.intervalBeatRejectCount),
      static_cast<unsigned long>(current.max30102.beatReseedCount),
      static_cast<unsigned long>(current.max30102.unstableBeatRejectCount),
      static_cast<unsigned long>(current.max30102.falseBeatGateCount),
      BPMWATCH_MAX30102_WRIST_MODE ? 1 : 0,
      static_cast<long>(BPMWATCH_MAX30102_FINGER_IR_MIN),
      BPMWATCH_MAX30102_BPM_MIN,
      BPMWATCH_MAX30102_BPM_MAX,
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
#if BPMWATCH_NEEDS_I2C
  beginI2cBusSync();
#endif
  beginUwbEventQueue();

#if BPMWATCH_NEEDS_I2C
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
  {
    I2cBusLock i2cLock(BPMWATCH_I2C_LOCK_TIMEOUT_MS);
    if (i2cLock.acquired()) {
      scanI2cBus(Wire, state.compass);
    } else {
      ++state.compass.i2cLockFailCount;
    }
  }
  Serial.printf("I2C_DEVICES=%s\n", state.compass.i2cDeviceList);
#else
  Serial.println("I2C_DEVICES=OFF");
#endif

#if BPMWATCH_ENABLE_COMPASS
  {
    I2cBusLock i2cLock(BPMWATCH_COMPASS_I2C_LOCK_TIMEOUT_MS);
    if (i2cLock.acquired()) {
      compass.begin(Wire, state.compass);
    } else {
      ++state.compass.i2cLockFailCount;
      state.compass.lastIssue = CompassIssue::LockTimeout;
    }
  }
  Serial.printf(
      "COMPASS init: %s CSTAT=%s C_ERR=%s MAG=%s MAG_ADDR=%s MAG_DRIVER=%s MAG_WHOAMI=0x%02X ACC=%s ACC_ADDR=%s ACC_DRIVER=%s COMPASS_MODE=%s HEADING_MODE=%s C_LOCK_FAIL=%lu C_READ_FAIL=%lu C_WRITE_FAIL=%lu C_LAST_FAIL_STAGE=%s\n",
                state.compass.initialized
                    ? (state.compass.accelAvailable ? "OK" : "PARTIAL")
                    : "FAIL",
                state.compass.initialized ? "OK" : "ERR",
                compassIssueLabel(state.compass),
                compassMagLabel(state.compass),
                compassMagAddressLabel(state.compass),
                compassMagDriverLabel(state.compass),
                static_cast<unsigned>(state.compass.magWhoAmI),
                compassAccelLabel(state.compass),
                compassAccelAddressLabel(state.compass),
                compassAccelDriverLabel(state.compass),
                compassModeLabel(state.compass),
                compassHeadingModeLabel(state.compass),
                static_cast<unsigned long>(state.compass.i2cLockFailCount),
                static_cast<unsigned long>(state.compass.i2cReadFailCount),
                static_cast<unsigned long>(state.compass.i2cWriteFailCount),
                compassFailStageLabel(state.compass));
#else
  Serial.println("COMPASS: OFF");
#endif

#if BPMWATCH_ENABLE_MAX30102
  tryBeginMax30102(Wire, true);
#else
  Serial.println("MAX30102: OFF");
#endif

#if BPMWATCH_ENABLE_SOS
  pinMode(BPMWATCH_SOS_BUTTON_PIN, INPUT_PULLUP);
  Serial.printf("SOS button: pin=%d active=LOW debounce=%lums long=%lums\n",
                BPMWATCH_SOS_BUTTON_PIN,
                static_cast<unsigned long>(BPMWATCH_SOS_DEBOUNCE_MS),
                static_cast<unsigned long>(BPMWATCH_SOS_LONG_PRESS_MS));
#else
  Serial.println("SOS button: OFF");
#endif

#if BPMWATCH_ESPNOW_RANGE_LINK
  Serial.printf("ESP-NOW range link: %s\n",
                espNowRange.begin(state) ? "OK" : "ERROR");
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

#if BPMWATCH_ENABLE_COMPASS && BPMWATCH_COMPASS_TASK && defined(ARDUINO_ARCH_ESP32)
  const BaseType_t compassTaskCreateResult = xTaskCreatePinnedToCore(
      compassTask, "CompassTask", BPMWATCH_COMPASS_TASK_STACK, nullptr,
      BPMWATCH_COMPASS_TASK_PRIORITY, &compassTaskHandle,
      BPMWATCH_COMPASS_TASK_CORE);
  state.compass.compassTaskCreated = compassTaskCreateResult == pdPASS;
  Serial.printf(
      "COMPASS task: enabled=1 created=%d priority=%d core=%d stack=%d interval=%dms\n",
      state.compass.compassTaskCreated ? 1 : 0,
      BPMWATCH_COMPASS_TASK_PRIORITY, BPMWATCH_COMPASS_TASK_CORE,
      BPMWATCH_COMPASS_TASK_STACK, BPMWATCH_COMPASS_SAMPLE_INTERVAL_MS);
  if (!state.compass.compassTaskCreated) {
    Serial.println("COMPASS task warning: task create failed; loop fallback will sample compass");
  }
#endif

#if BPMWATCH_ESPNOW_RANGE_LINK && BPMWATCH_ESPNOW_TASK && defined(ARDUINO_ARCH_ESP32)
  const BaseType_t espNowTaskCreateResult = xTaskCreatePinnedToCore(
      espNowTask, "EspNowTask", BPMWATCH_ESPNOW_TASK_STACK, nullptr,
      BPMWATCH_ESPNOW_TASK_PRIORITY, &espNowTaskHandle,
      BPMWATCH_ESPNOW_TASK_CORE);
  espNowTaskCreated = espNowTaskCreateResult == pdPASS;
  Serial.printf(
      "ESPNOW task: enabled=1 created=%d priority=%d core=%d stack=%d interval=%dms\n",
      espNowTaskCreated ? 1 : 0, BPMWATCH_ESPNOW_TASK_PRIORITY,
      BPMWATCH_ESPNOW_TASK_CORE, BPMWATCH_ESPNOW_TASK_STACK,
      BPMWATCH_ESPNOW_TX_INTERVAL_MS);
  if (!espNowTaskCreated) {
    Serial.println("ESPNOW task warning: task create failed; loop fallback will send snapshots");
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
#if BPMWATCH_ESPNOW_TASK && defined(ARDUINO_ARCH_ESP32)
  const bool espNowLoopFallback = !espNowTaskCreated;
#else
  const bool espNowLoopFallback = true;
#endif
  if (espNowLoopFallback &&
      safeAgeMs(nowMs, lastEspNowTxMs) >= BPMWATCH_ESPNOW_TX_INTERVAL_MS) {
    lastEspNowTxMs = nowMs;
    sendEspNowSnapshot(nowMs);
  }
#endif

  updateRadarState(
      {uwbSnapshot.distanceM, uwbSnapshot.hasRange && !uwbSnapshot.rangeStale,
       nowMs},
      state.radar);

#if BPMWATCH_ENABLE_COMPASS || BPMWATCH_ENABLE_MAX30102
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
    I2cBusLock i2cLock(BPMWATCH_I2C_LOCK_TIMEOUT_MS);
    if (i2cLock.acquired()) {
      max30102.sample(nowMs, state.max30102);
    } else {
      ++state.max30102.maxLockFailCount;
    }
  }
#endif
#endif

#if BPMWATCH_ENABLE_COMPASS
#if BPMWATCH_COMPASS_TASK && defined(ARDUINO_ARCH_ESP32)
  const bool sampleCompassInLoop = !state.compass.compassTaskCreated;
#else
  const bool sampleCompassInLoop = true;
#endif
  if (sampleCompassInLoop && nowMs - lastCompassMs >= sensorIntervalMs) {
    lastCompassMs = nowMs;
    sampleCompass(nowMs);
  }
#endif

#if BPMWATCH_ENABLE_SOS
  if (nowMs - lastSosPollMs >= BPMWATCH_SOS_POLL_INTERVAL_MS) {
    lastSosPollMs = nowMs;
    sampleSosButton(nowMs);
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
#if BPMWATCH_ENABLE_MAX30102
    updateMax30102BpmLostAlert(nowMs);
#endif
    display.render(copyDiagnosticsState(), nowMs, forceFullDisplayRefresh);
  }
#endif

#if BPMWATCH_ENABLE_COMPASS && BPMWATCH_COMPASS_CAL_LOG
  if (nowMs - lastCompassCalLogMs >= BPMWATCH_COMPASS_CAL_LOG_INTERVAL_MS) {
    lastCompassCalLogMs = nowMs;
    logCompassCalibrationSnapshot(copyDiagnosticsState().compass);
  }
#endif

  if (nowMs - lastLogMs >= 1000) {
    lastLogMs = nowMs;
#if BPMWATCH_ENABLE_MAX30102
    updateMax30102BpmLostAlert(nowMs);
#endif
    logDiagnostics(copyDiagnosticsState(), nowMs);
  }
}
