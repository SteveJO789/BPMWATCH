# Two-Node Radar MVP Test Plan

Current product nodes:

- Node A / DW1000 Anchor: `0C:8A:D3:7C:E5:A4`
- Node B / DW1000 Tag: `1C:75:C4:F4:E9:D4`

The diagnostic firmware may still print `MASTER / ANCHOR` and `SLAVE 1 / TAG`. Those strings map to Node A and Node B; they do not define an application Master/Slave architecture.

## Completed Module Bring-Up

- [x] ESP32 blink upload succeeds.
- [x] STA MAC addresses are recorded for the two current product boards.
- [x] ST7789 240x240 test displays text and map points on the tested wearable hardware.
- [x] I2C scanner detects GY-511 addresses `0x19` and `0x1E`.
- [x] I2C scanner detects MAX30102 address `0x57`.
- [x] MAX30102 reads raw IR.
- [x] MAX30102 BPM calculation firmware builds successfully.
- [x] BPM output has been compared with a reference heart-rate measurement.
- [x] LSM303DLHC reads acceleration, compass, and `0-359 deg` heading values.
- [x] Protected TP4056 Type-C module charges the 602030 Li-Po safely.

## Node Hardware Parity

- [ ] Add ST7789, GY-511, and MAX30102 to Node A.
- [ ] Confirm Node A display pins `14/13/25/26/27`.
- [ ] Confirm Node A GY-511 and MAX30102 share I2C `21/22`.
- [ ] Confirm Node B uses the same current pin contract.
- [ ] Confirm both nodes remain stable with display, sensors, UWB, and ESP-NOW active.
- [ ] Confirm TPS63802 output during ESP32 radio and UWB bursts.
- [ ] Record GY-511 physical X/Y orientation relative to display top on both nodes.

## GY-511 Motion and Heading

- [x] Run `firmware/tests/i2c_scanner_test`.
- [x] Confirm accelerometer address `0x19`.
- [x] Confirm magnetometer address `0x1E`.
- [x] Confirm MAX30102 remains separate at `0x57` on the shared bus.
- [x] Run `firmware/tests/gy511_test`.
- [x] Confirm Serial Monitor prints `GY-511 init OK`.
- [ ] Tilt the module and confirm accelerometer X/Y/Z changes.
- [ ] Rotate the module and confirm magnetometer X/Y/Z changes.
- [x] Confirm heading stays in `0-359 deg`.
- [ ] Validate compile-time X/Y swap and sign mapping for Node A.
- [ ] Validate compile-time X/Y swap and sign mapping for Node B.
- [ ] Confirm weak acceleration does not change the movement bearing.
- [ ] Confirm clear forward/right acceleration produces the expected north-up bearing.

## UWB Pair Diagnostic

References:

- `docs/uwb-pair-ranging-ap-implementation-th.md`
- `docs/arduino-ide-uwb-pair-test.md`
- `docs/arduino-ide-uwb-pair-test-th.md`

- [x] Initialize B&T BU01 DW1000 LDO through default SPI.
- [x] Keep the no-CS ST7789 on a separate `SPIClass` bus.
- [x] Build diagnostic `master` and `tag` environments.
- [x] Confirm `BLINK -> RANGING_INIT -> POLL -> RANGE_REPORT`.
- [x] Add 5-second discovery and 15-second connected recovery.
- [x] Capture 1m raw range at antenna delay `16384` (approximately 2.48m).
- [x] Capture 1m raw range at antenna delay `16640` (approximately 0.26m).
- [x] Select calibrated antenna delay `16555` (approximately 0.94m at the calibration pass).
- [x] Capture unfiltered 2m and 5m results.
- [x] Enable the five-sample application median filter.
- [x] Pass filtered 5m test: 471 samples/60s, 4.999m average, no reconnect/recovery.
- [x] Accept 10m retest with limitation: 138 samples/30s, median 10.17m, one inactive/recovery.
- [x] Record 20m result: median 20.26m, 26 samples/30s, three recoveries.
- [x] Confirm Serial and diagnostic web monitor show live distance, RX power, and quality.
- [ ] Re-run Node A-Node B ranging after both nodes carry the complete Radar hardware load.
- [ ] Confirm production ranging works without AP/webserver code.

## Shared Radar Firmware

- [ ] Create `firmware/radar_node` with `node_a_anchor` and `node_b_tag` environments.
- [ ] Confirm both environments build from the same sources.
- [ ] Confirm Radar state updates every approximately 150 ms without blocking UWB polling.
- [ ] Confirm distance EMA initializes from the first valid range.
- [ ] Confirm cumulative distance change greater than `0.25m` triggers bearing evaluation.
- [ ] Confirm stable distance retains the previous Peer angle.
- [ ] Confirm angle smoothing crosses `0/360 deg` through the shortest path.

## ESP-NOW Peer Data

- [ ] Confirm Node A receives Node B BPM.
- [ ] Confirm Node B receives Node A BPM.
- [ ] Confirm invalid/no-finger BPM displays `PEER BPM:--`.
- [ ] Confirm 3-second ESP-NOW staleness changes only the BPM state.
- [ ] Confirm an accepted local bearing is sent immediately.
- [ ] Confirm the receiving node applies reciprocal `+180 deg` bearing.
- [ ] Confirm one moving node updates both displays.

## Radar Display

- [ ] Draw outer/inner circles, cross lines, north marker, and center dot.
- [ ] Show Node A and Node B labels correctly.
- [ ] Show an active Peer dot while UWB is current.
- [ ] Show `WAITING` and no dot before the first valid range.
- [ ] Keep the last dot as a gray outline after `UWB LOST`.
- [ ] Show active range as `R:5m`, `R:10m`, `R:20m`, or `R:40m`.
- [ ] Expand scale above 90% of the current band.
- [ ] Shrink scale only after 10 seconds below 35%.
- [ ] Freeze scale while UWB is lost.
- [ ] Show Peer BPM and distance at the bottom.
- [ ] Confirm both screens update without unacceptable flicker.

## Power and Field Tests

- [ ] Run each complete node on battery for 30 minutes.
- [ ] Run both nodes together for one hour.
- [ ] Check regulator temperature and ESP32 resets.
- [ ] Test one moving node at a time in open space.
- [ ] Test body obstruction, trees, walking, out-of-range, and recovery.
- [ ] Record simultaneous movement as an MVP limitation rather than an accuracy pass criterion.

Record evidence in:

- `test-logs/uwb-pair-test.md`
- `test-logs/radar-mvp-test.md`
- `test-logs/battery-test.md`
- `test-logs/field-test.md`
