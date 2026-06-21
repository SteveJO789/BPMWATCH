# GY-511 Bluetooth Monitor Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Mirror the GY-511 diagnostic output to Bluetooth Classic SPP while retaining USB Serial output and sensor operation when Bluetooth initialization fails.

**Architecture:** Keep the change inside the existing GY-511 test sketch. Add one Bluetooth transport and small line-output helpers so every diagnostic message reaches USB and, when available, Bluetooth without changing sensor register access.

**Tech Stack:** Arduino-ESP32, `BluetoothSerial`, PlatformIO, PowerShell regression check

---

### Task 1: Define the Bluetooth logging contract

**Files:**
- Create: `tools/verify-gy511-bluetooth.ps1`
- Test: `firmware/tests/gy511_test/src/main.cpp`

- [x] **Step 1: Write the failing source regression check**

Create a PowerShell script that verifies the sketch includes `BluetoothSerial.h`, enables SSP before starting `BPMWATCH-GY511`, omits legacy PIN configuration, and routes complete status and sample lines through a shared dual-output helper.

- [x] **Step 2: Run the check to verify it fails**

Run: `powershell -ExecutionPolicy Bypass -File tools/verify-gy511-bluetooth.ps1`

Expected: non-zero exit with a missing Bluetooth contract assertion.

### Task 2: Implement dual USB and Bluetooth output

**Files:**
- Modify: `firmware/tests/gy511_test/src/main.cpp`

- [x] **Step 1: Add minimal Bluetooth setup and output helpers**

Add `BluetoothSerial SerialBT`, a `bluetoothReady` state, and helpers that send each complete diagnostic line to USB and to SPP only when startup succeeded. Enable SSP before starting the device as `BPMWATCH-GY511`, and continue GY-511 initialization on Bluetooth failure.

- [x] **Step 2: Route existing status, errors, and sensor readings through the helper**

Format the existing sensor sample into a fixed local buffer and preserve the current fields, heading precision, and 500 ms interval.

- [x] **Step 3: Run the regression check**

Run: `powershell -ExecutionPolicy Bypass -File tools/verify-gy511-bluetooth.ps1`

Expected: `PASS: GY-511 Bluetooth diagnostic contract verified.`

### Task 3: Compile verification

**Files:**
- Verify: `firmware/tests/gy511_test/platformio.ini`

- [x] **Step 1: Build the GY-511 test environment**

Run: `C:\Users\pipat\.platformio\penv\Scripts\platformio.exe run -d firmware/tests/gy511_test`

Expected: `SUCCESS` for `esp32dev` with no compiler errors.

- [x] **Step 2: Review only task-owned changes**

Run: `git diff --check -- firmware/tests/gy511_test/src/main.cpp tools/verify-gy511-bluetooth.ps1 docs/superpowers/plans/2026-06-21-gy511-bluetooth-monitor.md`

Expected: no whitespace errors. Do not stage or alter unrelated worktree changes.
