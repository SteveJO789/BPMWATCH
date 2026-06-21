# Integrated Node Diagnostics Design

Date: 2026-06-21

## Status

Approved design for implementation.

## Goal

Create a two-node hardware integration test that runs the ST7789 display, GY-511, MAX30102, and the proven DW1000 pair-ranging flow together. The 240x240 display replaces Bluetooth as the primary field diagnostic output.

This phase validates complete-node operation before Radar map geometry, ESP-NOW, and Peer BPM are introduced.

## Scope

- Create `firmware/tests/integrated_node_test` without changing the focused module tests.
- Build the same source as `node_a_anchor` and `node_b_tag`.
- Display live local hardware values only; do not use mock values.
- Preserve the current two-node ranging roles, calibration, median filter, callbacks, and recovery behavior from `firmware/tests/uwb_pair_test`.
- Use the user's current MAX30102 beat-detection behavior, including the four-sample average.
- Use the confirmed GY-511 register configuration and heading calculation from `firmware/tests/gy511_test`.
- Use the confirmed ZJY-IPS130-V2.0 ST7789 initialization from `firmware/tests/st7789_display_test`.

## Non-Goals

- Radar map rendering or bearing estimation
- ESP-NOW or Peer BPM exchange
- Bluetooth Classic, BLE, Wi-Fi AP, webserver, or browser monitoring
- Changes to the focused UWB, GY-511, MAX30102, or display test sketches
- Reuse of the obsolete three-node `firmware/slave` or `firmware/master` architecture

## Repository Shape

```text
firmware/tests/integrated_node_test/
|-- platformio.ini
`-- src/
    |-- main.cpp
    |-- NodeConfig.h
    |-- DiagnosticsState.h
    |-- DiagnosticsDisplay.h
    |-- DiagnosticsDisplay.cpp
    |-- Gy511Sensor.h
    |-- Gy511Sensor.cpp
    |-- Max30102Sensor.h
    |-- Max30102Sensor.cpp
    |-- UwbDiagnostics.h
    `-- UwbDiagnostics.cpp
```

Each module owns one hardware responsibility. `main.cpp` initializes the shared buses and runs the non-blocking scheduler.

## Build Environments

`node_a_anchor` defines Node A and the fixed DW1000 Anchor radio role:

- Node label: `NODE A / ANCHOR`
- EUI: `0C:8A:D3:7C:E5:A4:00:01`
- Antenna delay: `16555`

`node_b_tag` defines Node B and the fixed DW1000 Tag radio role:

- Node label: `NODE B / TAG`
- EUI: `1C:75:C4:F4:E9:D4:00:01`
- Antenna delay: `16555`

Anchor and Tag are radio roles only. This test does not introduce an application master.

## Hardware And Buses

The shared pin contract is:

| Device | Interface | Pins |
|---|---|---|
| BU01/DW1000 | Default SPI | SCK 18, MISO 19, MOSI 23, CS 5, IRQ 34, RST 4 |
| ST7789 no-CS | Dedicated HSPI | SCK 14, MOSI 13, BLC 25, DC 26, RST 27 |
| GY-511 | Shared I2C | SDA 21, SCL 22 |
| MAX30102 | Shared I2C | SDA 21, SCL 22 |

`Wire.begin(21, 22)` is called once. GY-511 uses addresses `0x19` and `0x1E`; MAX30102 uses `0x57`.

The no-CS display never shares the DW1000 SPI bus. Display startup uses:

1. Backlight high.
2. Manual reset high, low, high with 50 ms holds.
3. Dedicated HSPI startup.
4. `tft.init(240, 240, SPI_MODE3)`.
5. `tft.setSPISpeed(8000000)` after initialization.

## Module Responsibilities

### UwbDiagnostics

- Initialize default SPI and DW1000 communication.
- Confirm the printable device identifier begins with `DECA`.
- Use fixed Anchor or Tag role selected at compile time.
- Use antenna delay `16555` and `DW1000.MODE_LONGDATA_RANGE_ACCURACY`.
- Preserve the five-sample application median filter and disable the library range filter.
- Preserve new-range, new-device, blink where supported, and inactive-device callbacks.
- Preserve the current discovery and connected recovery behavior from `uwb_pair_test` without adding Wi-Fi responsibilities.
- Expose SPI readiness, Peer presence, filtered distance, range freshness, and recovery count to the display state.

### Gy511Sensor

- Initialize the accelerometer at `0x19` and magnetometer at `0x1E` with the confirmed register values.
- Sample without blocking the UWB loop.
- Expose initialization/read status, heading in `0-359.9` degrees, and raw accelerometer X/Y/Z values.
- A read failure changes only GY-511 state; it does not stop other modules.

### Max30102Sensor

- Initialize through the shared `Wire` bus at fast I2C speed.
- Preserve the current LED setup and `checkForBeat()` behavior.
- Sample near every 20 ms without `delay(20)` in the main loop.
- Accept measured BPM from 40 through 220 and expose the four-valid-sample average.
- Expose initialization state, raw IR, average BPM, and finger presence using the current `IR >= 50000` threshold.
- Initialization failure changes only MAX30102 state.

### DiagnosticsDisplay

- Initialize the exact ST7789 module using the confirmed reset, mode, and speed sequence.
- Draw a fixed dashboard layout once and refresh only value regions to reduce flicker.
- Render green for valid operation, orange for waiting/no-finger states, and red for errors.
- Continue rendering available modules when another module fails.

## Dashboard

The single dashboard page shows:

- Header: `NODE A / ANCHOR` or `NODE B / TAG`
- UWB: `OK`, `WAIT`, `LOST`, or `SPI ERR`, plus filtered distance
- GY-511: `OK` or `READ ERR`, heading, and accelerometer X/Y/Z
- MAX30102: `OK`, `NO FINGER`, or `ERR`, raw IR, and average BPM

The display contains no Radar geometry, mock Peer, accuracy warning, or browser connection instructions.

## Scheduling

The loop remains non-blocking:

1. Poll `DW1000Ranging.loop()` continuously.
2. Update MAX30102 near every 20 ms.
3. Update GY-511 every 50 ms.
4. Refresh display values every 200 ms.
5. Emit a combined Serial diagnostic line every 1000 ms.

All timing uses elapsed `millis()` checks. Display and Serial cadence never controls sensor sampling or UWB polling.

## Failure Behavior

| Failure | Display behavior | Remaining system |
|---|---|---|
| DW1000 identifier is not `DECA` | `UWB SPI ERR`, distance `--` | Display and I2C sensors continue |
| Peer has not ranged yet | `UWB WAIT`, distance `--` | All local sensors continue |
| Last valid range is older than 3 seconds | `UWB LOST`, keep last distance visible | Recovery behavior remains active |
| GY-511 initialization or read fails | `GY READ ERR`, heading/accel `--` | UWB, MAX30102, display continue |
| MAX30102 initialization fails | `MAX ERR`, IR/BPM `--` | UWB, GY-511, display continue |
| MAX30102 has no finger | `MAX NO FINGER`, BPM `--` | Raw IR and other modules continue |

## Verification

Automated/source checks verify:

- Both PlatformIO environments exist with the correct role and EUI flags.
- Display and DW1000 use separate SPI instances and the confirmed ST7789 initialization sequence.
- I2C is initialized once on GPIO21/22.
- Bluetooth, BLE, Wi-Fi AP, and webserver headers or startup calls are absent.
- The scheduler contains no `delay(20)` sensor loop.
- Both environments compile successfully.

Bench verification runs in this order:

1. Boot each node with its Peer off and confirm `UWB WAIT` while local sensors update.
2. Start the Peer and confirm filtered distance updates on both displays.
3. Rotate and tilt each node and confirm heading and acceleration change.
4. Place and remove a finger and confirm IR/BPM/no-finger behavior.
5. Interrupt each sensor or Peer separately and confirm error isolation.
6. Run both complete nodes together and watch for display flicker, stalled ranging, resets, and TPS63802 instability.
