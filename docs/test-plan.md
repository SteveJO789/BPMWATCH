# Test Plan

## Module Tests

- [x] ESP32 blink upload succeeds.
- [x] MAC address prints for all 3 boards.
- [x] ST7789 240x240 display shows text and simple map points.
- [ ] I2C scanner detects GY-511 addresses `0x19` and `0x1E`.
- [ ] I2C scanner detects MAX30102 address `0x57`.
- [x] MAX30102 reads raw IR.
- [x] MAX30102 BPM calculation firmware builds successfully.
- [ ] Verify BPM output against a reference heart-rate measurement.
- [x] LSM303DLHC reads acceleration and compass values.
- [ ] SOS button input changes state reliably.
- [ ] Battery ADC reads stable values.
- [x] TP4056 Type-C charger with protection charges the 602030 Li-Po safely.
- [ ] TPS63802 buck-boost output stays stable during ESP32 Wi-Fi/UWB bursts.

Current bring-up status, 2026-06-19:

- Phase 3.1 ESP32 board labeling is complete.
- Phase 3.2 ST7789 240x240 display test is complete.
- Phase 3.3 MAX30102 raw IR and BPM firmware implementation is complete; BPM accuracy still needs bench verification.
- Phase 3.4 GY-511 basic initialization and heading output work; remaining motion and multi-board checks are still open.
- Phase 3.5 UWB pair-ranging implementation and bench validation is the active milestone.
- Master MAC: `0C:8A:D3:7C:E5:A4`
- Slave 1 MAC: `1C:75:C4:F4:E9:D4`
- Slave 2 MAC: `FC:FA:31:FE:8C:E0`

## GY-511 / LSM303DLHC Sensor Test

- [x] Wire GY-511 VCC to 3.3V, GND to GND, SDA to GPIO21, and SCL to GPIO22.
- [ ] Run `firmware/tests/i2c_scanner_test`.
- [ ] Confirm accelerometer I2C address `0x19`.
- [ ] Confirm magnetometer I2C address `0x1E`.
- [ ] If MAX30102 is on the same bus, confirm it appears separately at `0x57`.
- [x] Run `firmware/tests/gy511_test`.
- [x] Confirm Serial Monitor prints `GY-511 init OK`.
- [ ] Tilt the module and confirm accelerometer X/Y/Z values change.
- [ ] Rotate the module and confirm magnetometer X/Y/Z values change.
- [x] Confirm heading prints in the expected `0-359 deg` range.
- [ ] Repeat the same test for the Slave 1 GY-511 module.
- [ ] Repeat the same test for the Slave 2 GY-511 module.
- [ ] Save any bad wiring, missing address, or unstable heading notes before integrating into slave firmware.

## UWB Pair Test

- Implementation guide in Thai: `docs/uwb-pair-ranging-ap-implementation-th.md`.
- Arduino IDE users can also follow `docs/arduino-ide-uwb-pair-test.md` or `docs/arduino-ide-uwb-pair-test-th.md`.
- [x] Initialize B&T BU01 DW1000 LDO through the ESP32 default SPI bus.
- [x] Keep the no-CS ST7789 on a separate `SPIClass` bus.
- [x] Implement one firmware source with Master/Anchor and Slave 1/Tag roles.
- [x] Build both PlatformIO environments: `master` and `tag`.
- [x] Master creates Wi-Fi AP `S.T.A.T-UWB` and serves the monitor at `http://192.168.4.1`.
- [x] Confirm `BLINK -> RANGING_INIT -> POLL -> RANGE_REPORT` works on the bench.
- [x] Add adaptive Master recovery: 5 seconds during discovery and 15 seconds after a connection.
- [x] Capture unfiltered raw ranges at 1m with antenna delay `16384` (stable raw range approximately 2.48m).
- [x] Measure the first calibration pass at 1m with antenna delay `16640` (stable raw range approximately 0.26m).
- [x] Measure the interpolated calibration pass at 1m with antenna delay `16555` (approximately 0.94m average).
- [x] Capture unfiltered raw ranges at 2m (approximately 2.32m average) and 5m (approximately 4.7m average).
- [x] Keep calibrated antenna delay `16555` and enable the application median filter with 5 samples.
- [x] Pass filtered 5m stability test: 471 samples/60s, 4.999m average, no reconnect or recovery.
- [x] Accept 10m retest with limitation: 138 samples/30s, median 10.17m, no outliers, one peer inactive/recovery.
- [x] Record 20m extended-range result: median 20.26m with only 26 samples/30s and three recoveries.
- [x] Upload Master/Anchor firmware to the Master ESP32.
- [x] Upload Slave 1/Tag firmware to the Slave 1 ESP32.
- [x] Confirm both Serial Monitors report a connected UWB peer.
- [x] Confirm real distance, RX power, and quality values update continuously.
- [x] Confirm the web monitor displays the same live ranging data.
- [x] Master <-> Slave 1 at 1m, 2m, 5m, 10m.
- [ ] Master <-> Slave 2 at 1m, 2m, 5m, 10m.
- [ ] Slave 1 <-> Slave 2 at 1m, 2m, 5m, 10m.
- [x] Document current calibration errors and quality observations in this checklist.

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
