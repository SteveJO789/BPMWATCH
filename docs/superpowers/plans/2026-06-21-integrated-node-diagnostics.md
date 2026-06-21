# Integrated Node Diagnostics Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a two-environment hardware integration test that continuously ranges with DW1000, samples GY-511 and MAX30102, and displays live diagnostics on the ST7789.

**Architecture:** A shared `DiagnosticsState` is written by three focused hardware modules and read by a display module. `main.cpp` owns one non-blocking scheduler; the default SPI bus belongs to DW1000, dedicated HSPI belongs to the no-CS display, and I2C is initialized once for both sensors.

**Tech Stack:** Arduino-ESP32 3.3.5, PlatformIO, vendored DW1000 library, SparkFun MAX3010x, Adafruit GFX/ST7789, PowerShell source checks, PlatformIO native tests

---

### Task 1: Add Failing Integration Contracts

**Files:**
- Create: `tools/verify-integrated-node-test.ps1`
- Create: `firmware/tests/integrated_node_test/test/test_median_range_filter.cpp`

- [ ] **Step 1: Write the failing source contract**

The PowerShell check must require both environments, the expected source modules, Node A/Node B roles, one `Wire.begin(21, 22)`, the confirmed ST7789 sequence, continuous UWB polling, 20/50/200/1000 ms scheduler intervals, and absence of `BluetoothSerial.h`, BLE, `WebServer.h`, `WiFi.h`, `softAP`, and `delay(20)`. It exits non-zero and prints every missing contract.

- [ ] **Step 2: Write the median filter native test**

Copy the proven assertions from `firmware/tests/uwb_pair_test/test/test_median_range_filter.cpp` and point the include at `../src/MedianRangeFilter.h`. The test covers invalid range rejection, a single outlier, and a real 10 m to 20 m step.

- [ ] **Step 3: Verify RED**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File tools/verify-integrated-node-test.ps1
```

Expected: FAIL because `firmware/tests/integrated_node_test` does not exist.

### Task 2: Create Shared Configuration And State

**Files:**
- Create: `firmware/tests/integrated_node_test/platformio.ini`
- Create: `firmware/tests/integrated_node_test/src/NodeConfig.h`
- Create: `firmware/tests/integrated_node_test/src/DiagnosticsState.h`
- Create: `firmware/tests/integrated_node_test/src/MedianRangeFilter.h`

- [ ] **Step 1: Define both build environments**

Use a shared environment with `lib_extra_dirs = ../../../lib` and these dependencies:

```ini
lib_deps =
  sparkfun/SparkFun MAX3010x Pulse and Proximity Sensor Library
  adafruit/Adafruit GFX Library
  adafruit/Adafruit ST7735 and ST7789 Library
```

Set `BPMWATCH_NODE_ID=0` for `node_a_anchor` and `BPMWATCH_NODE_ID=1` for `node_b_tag`. Both use antenna delay `16555`, discovery recovery `5000`, and connected recovery `15000`.

Add a separate `[env:native]` using `platform = native` for the header-only median filter test; it does not compile Arduino sources.

- [ ] **Step 2: Define compile-time node identity**

`NodeConfig.h` exposes:

```cpp
struct NodeConfig {
  uint8_t nodeId;
  bool isAnchor;
  const char* displayLabel;
  const char* uwbEui;
};

inline constexpr NodeConfig kNodeConfig = /* selected by BPMWATCH_NODE_ID */;
```

Node A uses `NODE A / ANCHOR` and `0C:8A:D3:7C:E5:A4:00:01`; Node B uses `NODE B / TAG` and `1C:75:C4:F4:E9:D4:00:01`.

- [ ] **Step 3: Define shared state**

Create `UwbDiagnosticState`, `Gy511DiagnosticState`, `Max30102DiagnosticState`, and aggregate `DiagnosticsState`. Include readiness/error flags, range freshness fields, heading/accel, IR/finger/BPM, and initialize values to safe zero/false defaults.

- [ ] **Step 4: Copy the proven median filter**

Copy `MedianRangeFilter` unchanged from `firmware/tests/uwb_pair_test/src/MedianRangeFilter.h` so the native test verifies exactly the bench-tested behavior.

- [ ] **Step 5: Run the native test**

Run:

```powershell
C:\Users\pipat\.platformio\penv\Scripts\platformio.exe test -d firmware/tests/integrated_node_test -e native
```

Expected: all three median-filter assertions pass.

### Task 3: Implement Both I2C Sensor Modules

**Files:**
- Create: `firmware/tests/integrated_node_test/src/Gy511Sensor.h`
- Create: `firmware/tests/integrated_node_test/src/Gy511Sensor.cpp`
- Create: `firmware/tests/integrated_node_test/src/Max30102Sensor.h`
- Create: `firmware/tests/integrated_node_test/src/Max30102Sensor.cpp`

- [ ] **Step 1: Implement GY-511 without owning `Wire.begin()`**

Expose:

```cpp
class Gy511Sensor {
 public:
  bool begin(TwoWire& wire);
  void sample(Gy511DiagnosticState& state);
};
```

Reuse addresses `0x19`/`0x1E`, confirmed register values, byte ordering, and normalized `atan2f(my, mx)` heading from `gy511_test`. A failed read sets `readOk=false` and preserves process liveness.

- [ ] **Step 2: Implement MAX30102 without blocking**

Expose:

```cpp
class Max30102Sensor {
 public:
  bool begin(TwoWire& wire);
  void sample(uint32_t nowMs, Max30102DiagnosticState& state);
};
```

Reuse the current MAX30102 LED setup, `checkForBeat`, 40-220 BPM validation, four-value average, and `IR >= 50000` finger threshold. Do not call `delay()`.

- [ ] **Step 3: Compile-check both modules through the full environments**

Run both `node_a_anchor` and `node_b_tag` builds after `main.cpp` is available in Task 6; until then use the source contract to verify neither module calls `Wire.begin()` or `delay(20)`.

### Task 4: Implement UWB Diagnostics

**Files:**
- Create: `firmware/tests/integrated_node_test/src/UwbDiagnostics.h`
- Create: `firmware/tests/integrated_node_test/src/UwbDiagnostics.cpp`

- [ ] **Step 1: Define the callback-owning module**

Expose:

```cpp
class UwbDiagnostics {
 public:
  void begin(UwbDiagnosticState& state);
  void poll(uint32_t nowMs, UwbDiagnosticState& state);
 private:
  void restart(UwbDiagnosticState& state);
};
```

Use a single static instance/state bridge for the C-style `DW1000Ranging` callbacks.

- [ ] **Step 2: Port proven pair ranging without network code**

Keep pins `18/19/23/5/34/4`, antenna delay `16555`, `MODE_LONGDATA_RANGE_ACCURACY`, five-sample median filtering, printable `DECA` identifier check, range/RX/quality callbacks, device/inactive callbacks, Anchor blink callback, and current 5 s/15 s Anchor recovery behavior. Do not include Wi-Fi or webserver code.

- [ ] **Step 3: Expose display-safe state**

Update filtered distance and `lastRangeMs` only for accepted samples. Mark a range lost when its age exceeds 3000 ms while preserving the last distance.

### Task 5: Implement The ST7789 Dashboard

**Files:**
- Create: `firmware/tests/integrated_node_test/src/DiagnosticsDisplay.h`
- Create: `firmware/tests/integrated_node_test/src/DiagnosticsDisplay.cpp`

- [ ] **Step 1: Implement confirmed display initialization**

Own `SPIClass(HSPI)` and `Adafruit_ST7789` with no CS. Use pins `14/13/25/26/27`, manual reset holds, `init(240, 240, SPI_MODE3)`, and `setSPISpeed(8000000)` after init.

- [ ] **Step 2: Draw a fixed dashboard**

Draw the Node/role header and three bordered panels once. Provide `render(const DiagnosticsState&, uint32_t nowMs)` that clears only panel value rectangles and uses green for valid, orange for wait/no-finger, and red for errors.

- [ ] **Step 3: Render required values**

UWB shows state and filtered distance; GY-511 shows state, heading, and accel X/Y/Z; MAX30102 shows state, IR, and averaged BPM. Use `--` for unavailable values and preserve the last distance with `LOST` after 3 seconds.

### Task 6: Integrate The Non-Blocking Scheduler

**Files:**
- Create: `firmware/tests/integrated_node_test/src/main.cpp`

- [ ] **Step 1: Initialize buses and modules in ownership order**

Initialize Serial, call `Wire.begin(21, 22)` once, initialize the display, both I2C modules, then UWB/default SPI. Each init result updates only its module state.

- [ ] **Step 2: Implement elapsed-time scheduling**

The loop must:

```cpp
uwb.poll(nowMs, state.uwb);                    // every pass
if (nowMs - lastMaxMs >= 20) { max30102.sample(nowMs, state.max30102); }
if (nowMs - lastGyMs >= 50) { gy511.sample(state.gy511); }
if (nowMs - lastDisplayMs >= 200) { display.render(state, nowMs); }
if (nowMs - lastLogMs >= 1000) { logDiagnostics(state, nowMs); }
```

There is no Bluetooth, Wi-Fi, webserver, mock data, or blocking sensor delay.

- [ ] **Step 3: Verify GREEN source contract**

Run:

```powershell
powershell -ExecutionPolicy Bypass -File tools/verify-integrated-node-test.ps1
```

Expected: `PASS: integrated node diagnostics contract verified.`

### Task 7: Build And Review

**Files:**
- Verify: `firmware/tests/integrated_node_test/**`
- Verify: `tools/verify-integrated-node-test.ps1`

- [ ] **Step 1: Run the native median-filter test**

```powershell
C:\Users\pipat\.platformio\penv\Scripts\platformio.exe test -d firmware/tests/integrated_node_test -e native
```

Expected: PASS.

- [ ] **Step 2: Build Node A**

```powershell
C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -d firmware/tests/integrated_node_test -e node_a_anchor
```

Expected: SUCCESS.

- [ ] **Step 3: Build Node B**

```powershell
C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -d firmware/tests/integrated_node_test -e node_b_tag
```

Expected: SUCCESS.

- [ ] **Step 4: Review task-owned changes**

Run `git diff --check` and inspect only the new integrated test, verifier, and this plan. Do not stage or alter unrelated existing worktree changes.
