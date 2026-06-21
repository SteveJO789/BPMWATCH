# Radar 2D Map MVP Design

Date: 2026-06-21

## Status

Approved design for implementation.

## Goal

Replace the product's three-node triangle-map architecture with a two-node wearable radar MVP. Each node places itself at the center of a north-up ST7789 display and shows the other node as a dot using UWB distance plus a movement-based bearing estimate.

The MVP prioritizes useful visual behavior over positioning accuracy.

## Product Decisions

- The product has exactly two equal Radar Nodes. There is no application-level master.
- Node A uses STA MAC `0C:8A:D3:7C:E5:A4` and the fixed DW1000 Anchor radio role.
- Node B uses STA MAC `1C:75:C4:F4:E9:D4` and the fixed DW1000 Tag radio role.
- Anchor and Tag are radio roles only. Product UI and documentation use Node A, Node B, current node, and Peer.
- Both nodes use ESP32, B&T BU01/DW1000, GY-511/LSM303DLHC, MAX30102, and a no-CS ST7789 240x240 display.
- The existing working two-node DW1000 ranging flow, calibration, filtering, and recovery behavior remain the baseline.
- ESP-NOW carries Peer BPM and accepted bearing estimates. It does not create a Wi-Fi AP or browser UI.
- The existing `firmware/tests/uwb_pair_test` remains an unchanged diagnostic test, including its AP monitor.
- The superseded three-node production code and related artifacts move under a clearly marked legacy location during implementation.

## Non-Goals

- Three-node ranging or triangle solving
- `TeamMapPacket` or shared team-map broadcasting
- True 2D positioning
- DW1000 Angle-of-Arrival
- GPS coordinates
- Wi-Fi AP, webserver, or browser dashboard in production firmware
- Dynamic Anchor/Tag role switching
- High-accuracy inertial dead reckoning
- Accuracy warning UI
- OLED support

## Repository Shape

The canonical production firmware will be:

```text
firmware/radar_node/
|-- platformio.ini
`-- src/
    |-- main.cpp
    |-- NodeConfig.h
    |-- UwbPeerRanging.h
    |-- UwbPeerRanging.cpp
    |-- Gy511Motion.h
    |-- Gy511Motion.cpp
    |-- HeartRateSensor.h
    |-- HeartRateSensor.cpp
    |-- PeerLink.h
    |-- PeerLink.cpp
    |-- RadarMap.h
    `-- RadarMap.cpp
```

PlatformIO provides two environments from the same source:

- `node_a_anchor`
- `node_b_tag`

Build flags select the node ID, Peer STA MAC, and fixed DW1000 role. Node behavior outside the radio role is symmetric.

## Hardware and Buses

Both nodes use the same pin contract:

| Device | Interface | ESP32 pins |
|---|---|---|
| BU01/DW1000 | Default SPI | SCK 18, MISO 19, MOSI 23, CS 5, IRQ 34, RST 4 |
| ST7789 no-CS | Dedicated HSPI | SCK 14, MOSI 13, BLC 25, DC 26, RST 27 |
| GY-511 | I2C | SDA 21, SCL 22 |
| MAX30102 | Shared I2C | SDA 21, SCL 22 |

The display must remain on its dedicated `SPIClass`; the no-CS ST7789 must not share the DW1000 bus.

## Core Data Structures

The Radar module starts with the required public contracts:

```cpp
struct RadarInput {
  float distanceM;
  float headingDeg;
  float accelX;
  float accelY;
  float accelZ;
  bool linkOk;
  unsigned long timestamp;
};

struct RadarState {
  float peerAngleDeg;
  float peerDistanceM;
  float lastDistanceM;
  float smoothedDistanceM;
  bool hasAngle;
  bool hidePeerDot;
  unsigned long lastUpdate;
};
```

Internal state may extend `RadarState` or remain private to `RadarMap` for movement filtering, distance baseline, auto-range hysteresis, last valid link state, and accepted-bearing publication. Peer BPM and ESP-NOW freshness remain outside `RadarInput` because they do not affect radar geometry.

## Sensor Sampling

- MAX30102 sampling remains near 20 ms so beat detection is not slowed by UI or packet timing.
- GY-511 is sampled independently from the 150 ms display cadence.
- GY-511 and MAX30102 share I2C without blocking the UWB ranging loop.
- Configurable compile-time axis mapping supports X/Y swap and sign inversion.
- The initial mounting convention is mapped `+Y = display top`.

## Movement Bearing Heuristic

The GY-511 does not contain a gyroscope, so the MVP does not claim reliable inertial navigation. It uses a short-lived horizontal-acceleration direction as a visual movement heuristic:

1. Apply axis mapping.
2. Low-pass accelerometer axes to estimate gravity.
3. Subtract gravity to obtain approximate linear acceleration.
4. Reject weak acceleration as stationary/noisy.
5. For accepted motion, compute the relative direction with `atan2(rightAccel, forwardAccel)`.
6. Add compass heading and normalize to produce a north-up movement bearing.
7. Keep the last accepted movement bearing while acceleration becomes weak.

The first implementation uses test-tunable constants for the gravity filter, motion threshold, confirmation samples, and motion hold time. Bench tests, not theoretical precision, determine their final values.

## Distance and Bearing Update

For each valid UWB distance:

```cpp
smoothedDistanceM = smoothedDistanceM * 0.75f + distanceM * 0.25f;
```

The first valid sample initializes the EMA directly to avoid ramping from zero.

The distance delta is cumulative since the last accepted bearing update, not merely the difference between adjacent 100-200 ms samples:

```cpp
delta = smoothedDistanceM - lastDistanceM;
```

When recent motion is valid and `abs(delta) > 0.25m`:

- `delta < 0`: target Peer angle is the movement bearing.
- `delta > 0`: target Peer angle is the movement bearing plus 180 degrees.
- Smooth toward the target with shortest-path angular smoothing and alpha `0.15`.
- Set the new smoothed distance as the next bearing baseline.
- Publish the accepted bearing to the Peer through ESP-NOW.

If no angle exists after the first valid range, initialize `peerAngleDeg` from the current compass heading. If distance change is below the threshold, keep the previous Peer angle.

Required helpers are:

- `normalizeAngle(float angle)`
- `angleDiff(float a, float b)`
- `smoothAngle(float current, float target, float alpha)`
- `mapDistanceToRadius(float distanceM)`
- `updateRadarState(RadarInput input)`
- `drawRadarMap()`

## Reciprocal Bearing

When one node accepts a new Peer bearing `theta`, it sends that estimate through ESP-NOW. The receiving node stores `normalizeAngle(theta + 180 degrees)` so both displays update even when only one node is moving.

The MVP primarily supports one moving node at a time. If both move and produce conflicting estimates, the newest locally received estimate wins; temporary dot movement is accepted as an MVP limitation.

## Auto-Range

The radar uses discrete ranges:

```text
5 m -> 10 m -> 20 m -> 40 m
```

- Expand immediately when smoothed distance exceeds 90% of the current range.
- Shrink one or more bands only after distance remains below 35% for 10 seconds.
- Do not change range while UWB is lost.
- Display the active range as `R:5m`, `R:10m`, `R:20m`, or `R:40m`.

The radial conversion is:

```cpp
radiusPx = constrain(
    (smoothedDistanceM / currentRangeM) * radarRadiusPx,
    5.0f,
    radarRadiusPx);
```

## Screen Geometry and Rendering

The only supported display is the no-CS ST7789 240x240:

- Center: `(120, 120)`
- Radar radius: `100 px`
- North-up orientation
- Angle `0 degrees`: top
- Angle `90 degrees`: right

Polar conversion is:

```cpp
angleRad = peerAngleDeg * DEG_TO_RAD;
peerX = centerX + radiusPx * sinf(angleRad);
peerY = centerY - radiusPx * cosf(angleRad);
```

Each frame contains:

- Outer radar circle
- Inner half-range circle
- Horizontal and vertical cross lines
- `N` marker
- Green center dot for the current node
- Orange filled Peer dot when UWB is valid
- Gray outline Peer dot at the last known position when UWB is lost
- `WAITING` and no Peer dot before the first valid range
- Node label, UWB status, and active range at the top
- Peer BPM and distance at the bottom

The screen updates every 150 ms using one display SPI transaction per frame. Production rendering has no accuracy warning screen.

## UWB Ranging

Production code reuses the proven behavior from `firmware/tests/uwb_pair_test`:

- Fixed Anchor and Tag roles
- Antenna delay `16555`
- Application median filter with five samples
- Raw range, RX power, and quality callbacks
- Discovery recovery after 5 seconds
- Post-connection recovery after 15 seconds

The production module removes AP/webserver responsibilities. The diagnostic test retains them.

If no valid range arrives for 3 seconds, UI state becomes `UWB LOST`. The last known dot remains indefinitely until a new range arrives or the node restarts.

## ESP-NOW Peer Link

ESP-NOW is a bidirectional data link. It uses Wi-Fi station mode but does not create an AP.

```cpp
struct PeerStatusPacket {
  uint8_t version;
  uint8_t senderNodeId;
  uint16_t sequence;
  uint16_t bpm;
  float peerBearingDeg;
  uint8_t flags;
};
```

Flags indicate `bpmValid` and `bearingValid`.

- Send status every 500 ms.
- Send immediately after accepting a new bearing.
- Use local receive time for freshness rather than comparing unsynchronized node clocks.
- If no Peer packet arrives for 3 seconds, display `PEER BPM:--`.
- If MAX30102 has no valid finger/beat result, send `bpmValid=false`.
- UWB and ESP-NOW statuses are independent.

## Main Loop Scheduling

The loop remains non-blocking:

1. Poll DW1000 continuously.
2. Sample MAX30102 near every 20 ms.
3. Sample GY-511 at its configured sensor cadence.
4. Process ESP-NOW callbacks without blocking.
5. Update Radar state and render every 150 ms.
6. Send periodic Peer status every 500 ms.
7. Send accepted bearing events immediately.

No sensor sampling rate is reduced to control Serial output. Logs are throttled separately.

## Failure Behavior

| Failure | Radar behavior |
|---|---|
| No UWB range has ever arrived | No Peer dot; show `WAITING` |
| UWB range stale for 3 seconds | Keep last dot as gray outline; show `UWB LOST` |
| ESP-NOW stale for 3 seconds | Keep radar behavior; show `PEER BPM:--` |
| MAX30102 has no valid BPM | Send invalid BPM; Peer shows `PEER BPM:--` |
| GY-511 init/read failure | Keep distance and last angle; report sensor error through UI/Serial |
| Both nodes move simultaneously | Newest received bearing estimate wins; temporary instability is accepted |

## Documentation Migration

Implementation updates the current source of truth to the two-node architecture:

- `README.md`
- `docs/architecture.md`
- `docs/bom.md`
- `docs/pin-map.md`
- `docs/ranging-protocol.md`
- `docs/test-plan.md`
- wiring documentation
- main project plan

The new workflow diagram is `radar_2d_map_architecture.svg`. The old three-node SVG, triangle-map documentation, and production code move to a clearly named legacy area instead of being deleted.

## Verification

Automated tests cover:

- Angle normalization and wrap-around
- Shortest-path angle difference and smoothing
- EMA initialization and update
- Cumulative 0.25 m bearing baseline
- Motion gating and axis mapping
- Reciprocal bearing
- Auto-range hysteresis
- UWB-lost dot persistence
- ESP-NOW BPM freshness

Build gates are:

- `node_a_anchor`
- `node_b_tag`
- diagnostic `uwb_pair_test` master
- diagnostic `uwb_pair_test` tag

Bench verification covers:

1. Existing UWB pair ranging behavior
2. GY-511 axis mapping
3. Node A moving while Node B remains still
4. Node B moving while Node A remains still
5. Decreasing and increasing distance behavior
6. Bearing persistence when distance is stable
7. UWB loss with persistent inactive dot
8. ESP-NOW loss with invalid Peer BPM only
9. Bidirectional Peer BPM display
10. Auto-range bands and hysteresis

Results are stored in a new Radar MVP test log.
