# BPMWATCH

BPMWATCH is a two-node wearable Radar MVP built with ESP32, B&T BU01/DW1000 ranging, GY-511 heading and motion sensing, MAX30102 heart-rate sensing, ESP-NOW Peer data, and a no-CS ZJY-IPS130-V2.0 ST7789 240x240 display.

Each node places itself at the center of a north-up radar screen and shows the other node as a distance-scaled dot. The direction is a movement-based visual estimate, not a true UWB angle measurement.

## Current Architecture

The product has two equal Radar Nodes:

| Product node | ESP32 STA MAC | Fixed DW1000 role |
|---|---|---|
| Node A | `0C:8A:D3:7C:E5:A4` | Anchor |
| Node B | `1C:75:C4:F4:E9:D4` | Tag |

Anchor and Tag are radio protocol roles only. There is no application-level Master or Slave.

The two links have separate responsibilities:

- **UWB:** calibrated distance ranging only.
- **ESP-NOW:** Peer BPM and accepted reciprocal-bearing estimates.

Production firmware will not create a Wi-Fi AP, webserver, or browser dashboard. The existing AP monitor remains only in the diagnostic `uwb_pair_test`.

See:

- [Architecture workflow](radar_2d_map_architecture.svg)
- [Architecture](docs/architecture.md)
- [Radar MVP design spec](docs/superpowers/specs/2026-06-21-radar-2d-map-mvp-design.md)
- [Test plan](docs/test-plan.md)

## Hardware Per Node

- ESP32-WROOM-32
- B&T BU01 DW1000 LDO UWB breakout
- ZJY-IPS130-V2.0 IPS TFT LCD 240x240 ST7789 display without a CS pin
- GY-511 / LSM303DLHC accelerometer and compass
- MAX30102 heart-rate sensor
- 602030 Li-Po battery
- TP4056 Type-C charger with protection
- TPS63802 3.3V buck-boost supply
- Power switch, decoupling capacitors, and enclosure

Both nodes use the same buses and pin map. Only the MAC address and fixed DW1000 role differ.

## Radar Behavior

- North-up display with the current node at `(120,120)`.
- UWB distance is filtered with an EMA and mapped to an automatic `5/10/20/40m` range.
- GY-511 acceleration and compass heading provide a visual movement-bearing heuristic.
- Decreasing distance places the Peer along the estimated movement bearing.
- Increasing distance places the Peer approximately 180 degrees behind that bearing.
- ESP-NOW mirrors an accepted bearing to the other display with a reciprocal 180-degree transform.
- If UWB is lost, the last Peer dot remains as an inactive outline.
- If ESP-NOW is stale, radar ranging continues and Peer BPM displays `--`.

## Repository Map

- `firmware/radar_node`: approved production target; implementation pending.
- `firmware/tests/uwb_pair_test`: working two-node UWB diagnostic with AP monitor.
- `firmware/tests`: focused hardware bring-up projects.
- `firmware/master` and `firmware/slave`: deprecated pre-pivot production scaffolds; scheduled for legacy relocation during firmware implementation.
- `firmware/sim/wokwi`: legacy three-node UI simulation; not current Radar MVP validation.
- `lib/DW1000`: vendored ESP32-compatible DW1000 library.
- `docs/legacy/three-node`: superseded triangle-map documents and diagrams.
- `test-logs`: current two-node bench evidence.

## Current Status

Completed:

- ESP32 identity and STA MAC capture.
- ZJY-IPS130-V2.0 display bring-up using dedicated `SPIClass`, manual reset, `SPI_MODE3`, and 8 MHz display transactions.
- GY-511 I2C discovery, initialization, and heading output.
- MAX30102 raw IR and BPM calculation bring-up.
- Real Node A to Node B DW1000 ranging.
- Antenna delay `16555`, five-sample application median filter, and recovery behavior.
- Two-node Radar architecture, glossary, workflow SVG, and design specification.

Next implementation milestone:

1. Create the shared `firmware/radar_node` PlatformIO project.
2. Integrate UWB range callbacks without the production AP monitor.
3. Add GY-511 movement-bearing estimation and axis mapping.
4. Add ESP-NOW Peer BPM and reciprocal bearing.
5. Render and bench-test the radar on both ST7789 displays.

## Diagnostic Build

On this Windows machine, PlatformIO is available through:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e master -e tag
```

The diagnostic environments retain the literal names `master` and `tag` for compatibility with tested code. They map to Node A/Anchor and Node B/Tag.

## Electrical Safety

- Keep all ESP32, DW1000, ST7789, GY-511, and MAX30102 signal levels 3.3V-compatible.
- Confirm the exact BU01 breakout power-input label before applying power.
- Keep the no-CS ST7789 on dedicated HSPI; do not share the DW1000 default SPI bus.
- Initialize ZJY-IPS130-V2.0 with `tft.init(240, 240, SPI_MODE3)` and call `tft.setSPISpeed(8000000)` after init.
- Feed Li-Po voltage to an ESP32 ADC only through a safe resistor divider.
- Verify TPS63802 stability during ESP32 radio and UWB current bursts.

## Historical Architecture

The former one-Master/two-Slave triangle map is no longer the product direction. Its documentation is retained under [`docs/legacy/three-node`](docs/legacy/three-node/README.md).
