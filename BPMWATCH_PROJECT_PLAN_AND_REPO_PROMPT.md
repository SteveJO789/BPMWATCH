# BPMWATCH Two-Node Radar MVP Project Plan

## Product Goal

Build two equal wearable Radar Nodes. Each node displays itself at the center of a north-up ST7789 radar and shows the other node using calibrated UWB distance plus a movement-based bearing estimate.

The MVP targets clear visual behavior, not accurate 2D localization.

## Fixed Decisions

- Exactly two Radar Nodes: Node A and Node B.
- No application-level Master or Slave.
- Node A uses the fixed DW1000 Anchor radio role.
- Node B uses the fixed DW1000 Tag radio role.
- Both nodes have ESP32, BU01/DW1000, ST7789, GY-511, and MAX30102.
- UWB carries distance only.
- ESP-NOW carries Peer BPM and reciprocal-bearing estimates.
- Production firmware has no AP, webserver, or browser dashboard.
- The display is ST7789 240x240 only; OLED support is outside the MVP.
- The old three-node triangle architecture is legacy.

## Node Identity

| Node | STA MAC | DW1000 role | Product relationship |
|---|---|---|---|
| Node A | `0C:8A:D3:7C:E5:A4` | Anchor | Equal Radar Node |
| Node B | `1C:75:C4:F4:E9:D4` | Tag | Equal Radar Node |

## Hardware Contract

Each node contains:

- ESP32-WROOM-32
- B&T BU01 DW1000 LDO UWB breakout
- No-CS ST7789 240x240 display
- GY-511 / LSM303DLHC
- MAX30102
- 602030 Li-Po, protected TP4056 charger, TPS63802 3.3V supply, and power switch

Bus allocation:

- DW1000 default SPI: `SCK=18`, `MISO=19`, `MOSI=23`, `CS=5`, `IRQ=34`, `RST=4`
- ST7789 dedicated HSPI: `SCK=14`, `MOSI=13`, `BLC=25`, `DC=26`, `RST=27`
- Shared sensor I2C: `SDA=21`, `SCL=22`

## Production Firmware Target

```text
firmware/radar_node/
|-- platformio.ini
`-- src/
    |-- main.cpp
    |-- NodeConfig.h
    |-- UwbPeerRanging.h/.cpp
    |-- Gy511Motion.h/.cpp
    |-- HeartRateSensor.h/.cpp
    |-- PeerLink.h/.cpp
    `-- RadarMap.h/.cpp
```

One source tree builds two environments:

- `node_a_anchor`
- `node_b_tag`

## Radar Rules

- Update state and screen every 150 ms.
- Filter distance with `previous * 0.75 + measured * 0.25`.
- Compare distance against the baseline from the last accepted bearing update.
- Require cumulative distance change greater than `0.25m` plus detected movement.
- Use motion bearing when distance decreases and motion bearing plus 180 degrees when distance increases.
- Smooth angles through the shortest path with alpha `0.15`.
- Keep a previous angle when distance is stable.
- Use north-up coordinates with self at the center.
- Auto-range through `5m`, `10m`, `20m`, and `40m` bands.
- Keep the last dot indefinitely as an inactive outline after UWB loss.
- Show Peer BPM only; display `--` when ESP-NOW data is stale.

## Milestones

### 1. Hardware Bring-Up

- [x] Identify both ESP32 STA MAC addresses.
- [x] Validate no-CS ST7789 wiring and dedicated HSPI.
- [x] Detect GY-511 at `0x19` and `0x1E`.
- [x] Detect MAX30102 at `0x57`.
- [x] Calculate BPM from MAX30102 test firmware.
- [ ] Add ST7789, GY-511, and MAX30102 to Node A hardware.
- [ ] Confirm both nodes use identical current pin wiring.

### 2. UWB Pair Ranging

- [x] Range the current Node A/Anchor and Node B/Tag pair.
- [x] Calibrate antenna delay to `16555`.
- [x] Add five-sample application median filtering.
- [x] Validate adaptive recovery.
- [x] Record results from 1m through 20m.
- [ ] Reconfirm ranging after both nodes carry the complete display and sensor load.

### 3. Shared Radar Firmware

- [ ] Create `firmware/radar_node`.
- [ ] Build both node environments from common sources.
- [ ] Extract proven UWB behavior without production AP/web code.
- [ ] Keep `uwb_pair_test` unchanged as diagnostic tooling.

### 4. Motion Bearing

- [ ] Add configurable GY-511 axis mapping.
- [ ] Separate approximate gravity from horizontal acceleration.
- [ ] Detect movement and retain the last accepted movement bearing.
- [ ] Apply cumulative distance delta and angular smoothing.
- [ ] Document simultaneous movement as an accepted MVP limitation.

### 5. ESP-NOW Peer Data

- [ ] Exchange valid Peer BPM every 500 ms.
- [ ] Exchange accepted bearing estimates immediately.
- [ ] Apply reciprocal bearing on the receiving node.
- [ ] Track ESP-NOW freshness independently from UWB freshness.

### 6. Radar Display

- [ ] Draw outer and inner circles, cross lines, north marker, self dot, and Peer dot.
- [ ] Add automatic range bands with hysteresis.
- [ ] Show `UWB LOST` while preserving the last dot.
- [ ] Show `PEER BPM:--` when Peer data is stale.
- [ ] Verify both displays update without unacceptable flicker.

### 7. Power and Field Validation

- [ ] Validate TPS63802 output during UWB and ESP-NOW activity.
- [ ] Run each complete node on battery for 30 minutes.
- [ ] Run the pair for one hour.
- [ ] Test movement, body obstruction, trees, and link recovery.

## Acceptance Criteria

- Both nodes build from the same production source.
- UWB ranging continues with the calibrated pair behavior.
- Each display shows self at center and the Peer at distance-scaled coordinates.
- One moving node can update both screens through reciprocal bearing exchange.
- UWB loss preserves an inactive last-known dot.
- Peer BPM arrives over ESP-NOW in both directions.
- No production AP, webserver, or browser UI exists.

## Source of Truth

- `CONTEXT.md`
- `README.md`
- `radar_2d_map_architecture.svg`
- `docs/architecture.md`
- `docs/superpowers/specs/2026-06-21-radar-2d-map-mvp-design.md`
- `docs/test-plan.md`

Superseded three-node material is preserved under `docs/legacy/three-node/`.
