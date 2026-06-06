# BPMWATCH

BPMWATCH is a low-cost wearable team monitoring prototype using ESP32, BU01/DW1000 UWB ranging, BPM sensing, battery monitoring, and a shared relative 2D team map.

The current architecture follows `relative_2d_map_architecture.svg`: no GPS, no fixed anchors, 1 master node, 2 slave wearable nodes, and UWB distance measurements between all 3 node pairs.

## Relative 2D Map Concept

The master collects these distances:

- Master <-> Slave 1
- Master <-> Slave 2
- Slave 1 <-> Slave 2

From those 3 distances, the master solves a triangle and broadcasts the same `TeamMapPacket` back to every screen. This is a team-relative map, not a real-world GPS map.

## MVP Scope

- 1 Master Node
- 2 Slave/Wearable Nodes
- 3 BU01/DW1000 UWB modules
- 2 BPM sensors
- 2 ST7789 displays
- Shared relative 2D map on each slave screen

## Hardware Overview

Master:

- ESP32 DevKit / ESP32-WROOM-32
- BU01/DW1000 UWB module
- Li-Po battery, charger, regulator, switch

Each slave:

- ESP32 DevKit / ESP32-WROOM-32
- BU01/DW1000 UWB module
- ST7789 1.3 inch 240x240 SPI display
- MAX30102 BPM sensor
- Li-Po battery, charger, regulator, switch

## Firmware Overview

- `firmware/master`: collects slave status, manages UWB distances, solves the relative map, broadcasts `TeamMapPacket`.
- `firmware/slave`: reads BPM and battery, sends status, receives map packet, draws the team map.
- `firmware/tests`: small hardware validation projects.
- `tools/mac-address-scanner`: prints ESP32 MAC address for node setup.

## Build Instructions

1. Open this folder in VS Code.
2. Install the PlatformIO extension.
3. Open `firmware/tests/esp32_blink`.
4. Connect an ESP32 board.
5. Build and upload from PlatformIO.

Command-line example:

```bash
cd firmware/master
pio run
```

## Development Phases

1. Validate ESP32 boards and MAC addresses.
2. Validate ST7789 display and MAX30102 sensor.
3. Validate BU01/DW1000 pair ranging.
4. Validate 3-node UWB triangle ranging.
5. Build packet communication.
6. Build relative map solver.
7. Build slave display UI.
8. Build master broadcast flow.
9. Add battery and field testing.

## Safety Warning

BU01/DW1000 modules are 3.3V devices. Do not connect 5V directly to the UWB module.

## Current Status

Starter repository created from the project plan. Firmware contains placeholders where BU01 protocol details, real pins, and sensor calibration must be confirmed on hardware.

## TODO

- Confirm ESP32 board model.
- Confirm BU01 UART/SPI mode and protocol.
- Confirm actual pin mapping.
- Test all 3 UWB pair distances.
- Tune relative map smoothing.
