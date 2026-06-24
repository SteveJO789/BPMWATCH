# ESP-NOW Range Return Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Let Node A use the proven UWB Anchor range path and send measured distance back to Node B over ESP-NOW for display/radar rendering.

**Architecture:** Node A remains the UWB Anchor and becomes an ESP-NOW sender. Node B remains the UWB Tag for the DW1000 handshake and becomes an ESP-NOW receiver; its screen uses the latest Node A range packet as the display source. No Wi-Fi AP, WebServer, or browser UI is added.

**Tech Stack:** Arduino ESP32, DW1000Ranging, ESP-NOW, Adafruit ST7789, PlatformIO.

---

### Task 1: Add ESP-NOW Range Packet Module

**Files:**
- Create: `firmware/tests/integrated_node_test/src/EspNowRangeLink.h`
- Create: `firmware/tests/integrated_node_test/src/EspNowRangeLink.cpp`
- Test: `firmware/tests/integrated_node_test/test/test_espnow_range_packet.cpp`

- [x] **Step 1: Define a compact range packet with magic/version validation**

Create a packet containing sequence, distance, raw distance, RX power, quality, range count, and sender timestamp. Add native-testable validation helpers that reject wrong magic/version/size.

- [x] **Step 2: Implement ESP-NOW sender/receiver wrappers**

Use `WiFi.mode(WIFI_STA)`, `esp_now_init()`, and a fixed Node B STA MAC peer for Node A. On Node B, register a receive callback and update `UwbDiagnosticState` from valid packets.

### Task 2: Add ESP-NOW Build Environments

**Files:**
- Modify: `firmware/tests/integrated_node_test/platformio.ini`
- Modify: `tools/verify-integrated-node-test.ps1`

- [x] **Step 1: Add `node_a_anchor_espnow_range` and `node_b_tag_espnow_display`**

These envs use the same integrated firmware with `BPMWATCH_ESPNOW_RANGE_LINK=true`.

- [x] **Step 2: Update verification contract**

Require the new envs and ESP-NOW module, allow `WiFi.h` only for ESP-NOW while still forbidding `WebServer.h` and `softAP`.

### Task 3: Wire ESP-NOW Into Runtime

**Files:**
- Modify: `firmware/tests/integrated_node_test/src/UwbDiagnostics.cpp`
- Modify: `firmware/tests/integrated_node_test/src/main.cpp`
- Modify: `firmware/tests/integrated_node_test/src/DiagnosticsState.h`

- [x] **Step 1: Match pair-test recovery behavior**

Keep app-level UWB recovery on Node A/Anchor. Disable app-level recovery on Node B/Tag unless explicitly enabled by a build flag.

- [x] **Step 2: Send Node A range updates**

When `rangeCount` increases on Node A, send the latest range state over ESP-NOW.

- [x] **Step 3: Receive Node B display updates**

When Node B receives a valid ESP-NOW range packet, update `state.uwb` and let the existing radar/display path render it.

### Task 4: Verify

**Files:**
- Run: `tools/verify-integrated-node-test.ps1`
- Run: PlatformIO native tests
- Build: ESP-NOW Node A and Node B envs

- [x] **Step 1: Run native/unit checks**

Expected: packet helper tests and existing radar/GY/filter tests pass.

- [x] **Step 2: Build both uploadable ESP-NOW envs**

Expected: `node_a_anchor_espnow_range` and `node_b_tag_espnow_display` build successfully.
