# Test Plan

## Module Tests

- [x] ESP32 blink upload succeeds.
- [x] MAC address prints for all 3 boards.
- [ ] ST7789 240x240 display shows text and simple map points.
- [ ] I2C scanner detects GY-511 addresses `0x19` and `0x1E`.
- [ ] I2C scanner detects MAX30102 address `0x57`.
- [ ] MAX30102 reads raw IR and BPM.
- [ ] LSM303DLHC reads acceleration and compass values.
- [ ] SOS button input changes state reliably.
- [ ] Battery ADC reads stable values.
- [ ] TP4056 Type-C charger with protection charges the 602030 Li-Po safely.
- [ ] TPS63802 buck-boost output stays stable during ESP32 Wi-Fi/UWB bursts.

Current bring-up status, 2026-06-18:

- Phase 3.1 ESP32 board labeling is complete.
- Phase 3.2 ST7789 240x240 display test is the active milestone.
- Master MAC: `FC:FA:31:FE:8C:E0`
- Slave 1 MAC: `1C:75:C4:F4:E9:D4`
- Slave 2 MAC: `0C:8A:D3:7C:E5:A4`

## GY-511 / LSM303DLHC Sensor Test

- [ ] Wire GY-511 VCC to 3.3V, GND to GND, SDA to GPIO21, and SCL to GPIO22.
- [ ] Run `firmware/tests/i2c_scanner_test`.
- [ ] Confirm accelerometer I2C address `0x19`.
- [ ] Confirm magnetometer I2C address `0x1E`.
- [ ] If MAX30102 is on the same bus, confirm it appears separately at `0x57`.
- [ ] Run `firmware/tests/gy511_test`.
- [ ] Confirm Serial Monitor prints `GY-511 init OK`.
- [ ] Tilt the module and confirm accelerometer X/Y/Z values change.
- [ ] Rotate the module and confirm magnetometer X/Y/Z values change.
- [ ] Confirm heading prints in the expected `0-359 deg` range.
- [ ] Repeat the same test for the Slave 1 GY-511 module.
- [ ] Repeat the same test for the Slave 2 GY-511 module.
- [ ] Save any bad wiring, missing address, or unstable heading notes before integrating into slave firmware.

## UWB Pair Test

- Arduino IDE users should follow `docs/arduino-ide-uwb-pair-test.md` or Thai version `docs/arduino-ide-uwb-pair-test-th.md`.
- [ ] Master <-> Slave 1 at 1m, 2m, 5m, 10m.
- [ ] Master <-> Slave 2 at 1m, 2m, 5m, 10m.
- [ ] Slave 1 <-> Slave 2 at 1m, 2m, 5m, 10m.
- [ ] Log errors and quality values.

## UWB Triangle Test

- [ ] Place nodes in a known triangle.
- [ ] Measure all 3 pair distances.
- [ ] Confirm impossible triangles are rejected.
- [ ] Confirm stale pair makes map invalid.

## Map Display Test

- [ ] Master broadcasts `TeamMapPacket`.
- [ ] Slave 1 screen shows Master, S1, S2.
- [ ] Slave 2 screen shows the same map.
- [ ] BPM and battery fields are readable.

## Battery Test

- [ ] Run each node on battery for 30 minutes.
- [ ] Run 3-node system for 1 hour.
- [ ] Check regulator heat and ESP32 resets.

## Field Test

- [ ] Outdoor open space.
- [ ] Trees nearby.
- [ ] Human body blocking signal.
- [ ] Walking movement.
- [ ] Move out of range and return.
