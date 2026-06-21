# GY-511 Bluetooth Serial Monitor Design

## Goal

Allow the soldered ESP32 and GY-511 test assembly to be monitored from Windows after USB is disconnected and the board is powered from the TPS63802 3.3 V rail.

## Scope

- Modify only `firmware/tests/gy511_test/src/main.cpp`.
- Keep the existing GY-511 initialization, register reads, calculations, and 500 ms reporting interval.
- Keep USB `Serial` output available for bench diagnosis.
- Add Bluetooth Classic Serial Port Profile output for this diagnostic sketch only.
- Do not add Bluetooth to production Radar Node firmware and do not replace the planned ESP-NOW peer-data protocol.

## Runtime Design

The sketch creates an ESP32 `BluetoothSerial` endpoint named `BPMWATCH-GY511` using Secure Simple Pairing (SSP) in Just Works mode. Each startup, status, error, and sensor-data line is written to both USB `Serial` and Bluetooth serial.

Bluetooth startup failure must not block I2C setup or GY-511 sampling. The sketch reports the failure over USB and continues operating as the original USB-only sensor test.

## Data Flow

1. The ESP32 initializes USB serial.
2. It enables SSP before starting Bluetooth SPP.
3. It initializes I2C and the GY-511.
4. Every 500 ms, it reads accelerometer and magnetometer registers and calculates heading.
5. It formats one complete diagnostic line and sends the same line to both serial transports.

## Power And Connection Procedure

1. Isolate or switch off the TPS63802 output before connecting USB for upload so two regulators do not drive the same 3.3 V rail.
2. Upload the diagnostic firmware over USB.
3. Disconnect USB completely.
4. Restore TPS63802 power.
5. Pair Windows with `BPMWATCH-GY511`. No PIN is required.
6. Open the outgoing Bluetooth SPP COM port at 115200 baud. The SPP transport does not depend on UART baud electrically, but 115200 keeps terminal configuration consistent with the USB monitor.

## Verification

- The PlatformIO environment builds successfully.
- Source-level checks confirm the device name, SSP-before-startup ordering, absence of legacy PIN configuration, and dual-output behavior.
- With hardware, Windows can pair and receive GY-511 readings while USB is disconnected.
- If Bluetooth initialization fails, USB still reports the failure and GY-511 sampling continues.

## Non-Goals

- BLE support.
- A phone application or browser dashboard.
- Wi-Fi AP, webserver, or Telnet logging.
- Changes to production node-to-node communications.
