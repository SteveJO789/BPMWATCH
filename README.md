# BPMWATCH

BPMWATCH is a low-cost wearable team monitoring prototype using ESP32-WROOM-32, BU01 UWB ranging, BPM sensing, motion/compass sensing, battery monitoring, and a shared relative 2D team map.

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
- 3 B&T BU01 DW1000 LDO UWB breakout modules
- 2 BPM sensors
- 2 IPS TFT LCD 240x240 ST7789 display modules
- 2 LSM303DLHC accelerometer/compass modules
- SOS button and power switch on wearable nodes
- Shared relative 2D map on each slave screen

## Hardware Overview

Master:

- ESP32-WROOM-32
- B&T BU01 DW1000 LDO UWB breakout
- TP4056 Type-C charger with protection
- TPS63802 buck-boost module
- 602030 Li-Po battery
- Power switch and power-filter capacitors

Each slave:

- ESP32-WROOM-32
- B&T BU01 DW1000 LDO UWB breakout
- IPS TFT LCD 240x240 ST7789 display module with SCL/SCK, SDA/MOSI, BLC, DC, and RES pins
- LSM303DLHC accelerometer/compass sensor
- MAX30102 BPM sensor
- SOS button
- TP4056 Type-C charger with protection
- TPS63802 buck-boost module
- 602030 Li-Po battery
- Power switch and power-filter capacitors

## Firmware Overview

- `firmware/master`: collects slave status, manages UWB distances, solves the relative map, broadcasts `TeamMapPacket`.
- `firmware/slave`: reads BPM and battery, sends status, receives map packet, draws the team map.
- `firmware/tests`: small hardware validation projects.
- `firmware/sim/wokwi`: Wokwi simulation project with mocked UWB, BPM, battery, and GY-511 data.
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
2. Validate ST7789 240x240 display, GY-511/LSM303DLHC sensor, and MAX30102 sensor.
3. Validate BU01 UWB pair ranging.
4. Validate 3-node UWB triangle ranging.
5. Build packet communication.
6. Build relative map solver.
7. Build slave display UI.
8. Build master broadcast flow.
9. Add battery and field testing.

## Safety Warning

B&T BU01 DW1000 LDO UWB, the ST7789 240x240 display, GY-511/LSM303DLHC, and MAX30102 modules must use 3.3V-compatible signal levels. Confirm the BU01 breakout pin labels before using any regulator/VIN pin, and do not connect 5V directly to 3.3V-only pins.

## Current Status

Starter repository created from the project plan. Phase 3.1 ESP32 bring-up and MAC labeling are complete, and the project has reached Phase 3.2 ST7789 240x240 display testing.

Confirmed UWB hardware:

- B&T BU01 DW1000 LDO UWB breakout module
- Interface mode and exact pinout still need bench confirmation before final wiring.

Current node labels:

- Master: `FC:FA:31:FE:8C:E0`
- Slave 1: `1C:75:C4:F4:E9:D4`
- Slave 2: `0C:8A:D3:7C:E5:A4`

Firmware still contains placeholders where BU01 protocol details, real pins, and sensor calibration must be confirmed on hardware.

## TODO

- Confirm ESP32-WROOM-32 dev board/module variant.
- Confirm B&T BU01 DW1000 LDO UART/SPI mode, pinout, power input pin, and protocol.
- Confirm actual pin mapping.
- Confirm 602030 Li-Po capacity and discharge current.
- Test all 3 UWB pair distances.
- Tune relative map smoothing.
- Use no-CS ST7789 firmware mode; the physical display module exposes SCL, SDA, BLC, DC, and RES only.
- Run `firmware/tests/i2c_scanner_test` before GY-511 and MAX30102 driver tests.
