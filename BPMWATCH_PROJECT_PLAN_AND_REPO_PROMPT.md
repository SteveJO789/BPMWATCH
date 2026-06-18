# BPMWATCH Project Plan & Repository Creation Prompt

## Project Name

**BPMWATCH**  
A low-cost wearable team monitoring prototype using ESP32, BU01/DW1000 UWB ranging, BPM sensing, battery status, and a shared relative 2D team map.

---

## 1. Architecture Source

This project plan follows the architecture in:

```text
relative_2d_map_architecture.svg
```

Core idea from the SVG:

- No GPS
- No fixed anchor
- Every node can move
- Use UWB distance measurements between all 3 node pairs
- Master computes a relative 2D map
- Master sends the same map and team health data back to every screen

This is a **relative team map**, not real-world GPS coordinates.

---

## 2. Project Goal

Build a prototype system with:

- **1 Master Node**
- **2 Slave/Wearable Nodes**

Each node uses:

- ESP32
- BU01/DW1000 UWB module
- Battery power

Each slave should also have:

- BPM sensor
- Small screen

The master should:

- Collect BPM, battery, and node ID from slaves
- Measure or coordinate UWB ranging for all 3 pairs:
  - Master <-> Slave 1
  - Master <-> Slave 2
  - Slave 1 <-> Slave 2
- Compute a 2D triangle-style relative map
- Combine the relative map with BPM and battery status
- Broadcast the result back to every screen

Each screen should show:

- Relative 2D map
- Master position as the team reference point
- Slave 1 and Slave 2 positions based on measured UWB distances
- BPM and battery status for each node

---

## 3. Main Constraints

- Must be low-cost
- Must use components available in Thailand
- Must not use GPS
- Must not require fixed anchors in the first version
- Must use UWB for real distance measurement
- Must start with simple tabletop validation before wearable case design
- Must clearly document limitations of relative mapping

---

## 4. MVP Scope

The first full architecture prototype should support:

```text
1 Master Node + 2 Slave Nodes
```

This node count is required because the relative 2D map in the SVG depends on a triangle with 3 measured distances.

The MVP is considered successful when:

- Master <-> Slave 1 UWB distance works
- Master <-> Slave 2 UWB distance works
- Slave 1 <-> Slave 2 UWB distance works
- Slave 1 sends BPM, battery, and ID
- Slave 2 sends BPM, battery, and ID
- Master computes relative 2D positions
- Master broadcasts map and status data to all screens
- Each screen shows the same team overview

For early bring-up only, a temporary 1 master + 1 slave test is allowed for:

- ESP32 test
- BPM sensor test
- Display test
- Single UWB link test
- Packet communication test

But 1 master + 1 slave is **not enough** for the final relative 2D map.

---

## 5. Recommended Hardware

### 5.1 Master Node

| Component | Recommended Spec | Quantity |
|---|---:|---:|
| ESP32 DevKit / ESP32-WROOM-32 | 3.3V logic, Wi-Fi, 4MB flash or more | 1 |
| B&T BU01 DW1000 LDO UWB Module | Confirm UART/SPI mode, power input pin, and 3.3V-safe logic | 1 |
| Li-Po Battery | 3.7V, 1000-1500mAh | 1 |
| TP4056 Type-C Charger | With battery protection | 1 |
| 3.3V Regulator | ME6211 / AP2112 / buck-boost 3.3V | 1 |
| Power Switch | Mini slide switch | 1 |
| Capacitors | 470uF + 10uF + 0.1uF | 1 set |
| Case | ABS box or 3D printed case | 1 |

Optional:

| Component | Purpose |
|---|---|
| Small display | Local master screen |
| Button | Reset ranging / start calibration |
| Buzzer | Alert output for future warning states |

### 5.2 Slave/Wearable Node

Each slave node should contain:

| Component | Recommended Spec | Quantity per Slave |
|---|---:|---:|
| ESP32 DevKit / ESP32-WROOM-32 | 3.3V logic, Wi-Fi, 4MB flash or more | 1 |
| B&T BU01 DW1000 LDO UWB Module | Confirm UART/SPI mode, power input pin, and 3.3V-safe logic | 1 |
| IPS TFT LCD 240x240 ST7789 Display | Module labels SCL/SCK, SDA/MOSI, BLC, DC, and RES | 1 |
| MAX30102 Heart Rate Sensor | I2C heart-rate / pulse oximeter module | 1 |
| Li-Po Battery | 3.7V, 1000-1500mAh | 1 |
| TP4056 Type-C Charger | With battery protection | 1 |
| 3.3V Regulator | ME6211 / AP2112 / buck-boost 3.3V | 1 |
| Power Switch | Mini slide switch | 1 |
| Capacitors | 470uF + 10uF + 0.1uF | 1 set |
| Watch Strap | Velcro tactical strap | 1 |
| Case | 3D printed or small ABS case | 1 |

Future optional slave parts:

| Component | Purpose |
|---|---|
| SOS Button | Emergency alert feature after map MVP |
| Vibration Motor / Buzzer | Local alert output |
| Compass / IMU | Optional heading display, not required by the SVG architecture |

---

## 6. Important Technical Notes

### 6.1 UWB Power Warning

The BU01 / DW1000 module must use:

```text
3.3V only
```

Do not connect 5V directly to the UWB module.

Recommended power path:

```text
Li-Po 3.7V
  -> TP4056 charger
  -> Power switch
  -> Stable 3.3V regulator
  -> ESP32 + BU01 + sensors + display
```

### 6.2 Relative 2D Map Limitation

This system does not know true world position.

It only knows distances inside the team:

```text
M-S1 distance
M-S2 distance
S1-S2 distance
```

From these 3 distances, the master can draw a triangle. That triangle is useful for team-relative awareness, but it is not a GPS map and it is not an absolute compass-aligned map.

### 6.3 Display Choice

Use an **IPS TFT LCD 240x240 ST7789 display module** for the slave screen. The module exposes `SCL`, `SDA`, `BLC`, `DC`, and `RES`; on this module `SCL` is SPI `SCK` and `SDA` is SPI `MOSI`, not I2C.

The display should show:

- Relative map
- Node labels
- BPM values
- Battery values
- Signal/ranging state

### 6.4 Compass and SOS Scope

Compass and SOS are useful future features, but they are not part of the SVG architecture.

For this plan:

- Compass is optional and should not block MVP
- SOS is optional and should be added after relative map and BPM are working
- The first priority is UWB ranging between all 3 node pairs

---

## 7. Data Packet Structures

Use shared packet structures between master and slaves.

### 7.1 Slave Status Packet

```cpp
typedef struct {
  uint8_t id;
  int bpm;
  int battery;
  uint8_t signal;
} SlaveStatusPacket;
```

### 7.2 Distance Packet

```cpp
typedef struct {
  uint8_t fromId;
  uint8_t toId;
  float distanceMeters;
  uint32_t timestampMs;
  uint8_t quality;
} DistancePacket;
```

### 7.3 Team Map Packet

```cpp
typedef struct {
  float masterX;
  float masterY;
  float slave1X;
  float slave1Y;
  float slave2X;
  float slave2Y;
  int masterBattery;
  int slave1Bpm;
  int slave1Battery;
  int slave2Bpm;
  int slave2Battery;
  uint8_t mapValid;
} TeamMapPacket;
```

The master should broadcast `TeamMapPacket` so every screen shows the same view.

---

## 8. Recommended Repository Structure

```text
bpmwatch/
|-- README.md
|-- LICENSE
|-- .gitignore
|-- docs/
|   |-- architecture.md
|   |-- bom.md
|   |-- pin-map.md
|   |-- relative-2d-map.md
|   |-- ranging-protocol.md
|   |-- test-plan.md
|   |-- wiring-master.md
|   |-- wiring-slave.md
|   `-- limitations.md
|-- firmware/
|   |-- master/
|   |   |-- platformio.ini
|   |   `-- src/
|   |       |-- main.cpp
|   |       |-- Packets.h
|   |       |-- RelativeMapSolver.h
|   |       |-- RelativeMapSolver.cpp
|   |       |-- UwbRanging.h
|   |       |-- UwbRanging.cpp
|   |       |-- TeamBroadcaster.h
|   |       `-- TeamBroadcaster.cpp
|   |-- slave/
|   |   |-- platformio.ini
|   |   `-- src/
|   |       |-- main.cpp
|   |       |-- Packets.h
|   |       |-- DisplayUI.h
|   |       |-- DisplayUI.cpp
|   |       |-- HeartRateSensor.h
|   |       |-- HeartRateSensor.cpp
|   |       |-- BatteryMonitor.h
|   |       |-- BatteryMonitor.cpp
|   |       |-- UwbRanging.h
|   |       `-- UwbRanging.cpp
|   |-- tests/
|   |   |-- esp32_blink/
  |   |   |-- st7789_display_test/
|   |   |-- max30102_test/
|   |   |-- uwb_pair_test/
|   |   |-- uwb_triangle_test/
|   |   `-- broadcast_map_test/
|   `-- legacy/
|-- hardware/
|   |-- diagrams/
|   |-- case/
|   `-- photos/
|-- test-logs/
|   |-- uwb-pair-test.md
|   |-- uwb-triangle-test.md
|   |-- battery-test.md
|   `-- field-test.md
`-- tools/
    `-- mac-address-scanner/
```

---

## 9. Development Phases

## Phase 0 - Confirm Architecture

- [ ] Confirm no GPS
- [ ] Confirm no fixed anchor
- [ ] Confirm node count is 1 master + 2 slaves
- [ ] Confirm relative map is based on 3 UWB distances
- [ ] Confirm compass and SOS are future features, not MVP blockers
- [ ] Write limitations for relative 2D mapping

Output:

```text
Project scope matches relative_2d_map_architecture.svg
```

---

## Phase 1 - Prepare Development Environment

- [ ] Install VS Code
- [ ] Install PlatformIO
- [ ] Install ESP32 board support
- [ ] Create GitHub repository
- [ ] Create folder structure
- [ ] Create README.md
- [ ] Create docs folder
- [ ] Create firmware folder
- [ ] Create hardware folder
- [ ] Create test-logs folder

Output:

```text
Clean repository structure
```

---

## Phase 2 - Hardware Purchasing

- [ ] Buy 3 ESP32 boards
- [ ] Buy 3 BU01 UWB modules
- [ ] Buy 2 IPS TFT LCD 240x240 ST7789 display modules
- [ ] Buy 2 MAX30102 sensors
- [ ] Buy 3 Li-Po batteries
- [ ] Buy 3 TP4056 Type-C charger modules
- [ ] Buy 3 stable 3.3V regulators
- [ ] Buy power switches
- [ ] Buy capacitors
- [ ] Buy JST connectors
- [ ] Buy silicone wires
- [ ] Buy perfboards
- [ ] Buy watch straps
- [ ] Buy enclosure or prepare 3D printing

Confirmed UWB module type, 2026-06-18:

- B&T BU01 DW1000 LDO UWB breakout module
- Do not assume the interface mode from the DW1000 chip name alone; confirm the breakout UART/SPI mode and pin labels on the actual module.

Output:

```text
Enough hardware for 1 master + 2 slave relative map prototype
```

---

## Phase 3 - Test Each Module Separately

### 3.1 ESP32 Test

- [x] Flash blink firmware
- [x] Open Serial Monitor
- [x] Print MAC address
- [x] Label each board as Master, Slave 1, Slave 2
- [x] Save MAC addresses in docs/pin-map.md or docs/node-id.md

Bring-up note, 2026-06-18:

- Master MAC: `FC:FA:31:FE:8C:E0`
- Slave 1 MAC: `1C:75:C4:F4:E9:D4`
- Slave 2 MAC: `0C:8A:D3:7C:E5:A4`
- Current milestone reached: Phase 3.2 ST7789 240x240 display test.

### 3.2 ST7789 240x240 Display Test

- Status: in progress. ESP32 board identity and MAC labeling are complete; display wiring and visual checks are the active next work.
- [ ] Wire ST7789 240x240 display to ESP32
- [ ] Print text
- [ ] Draw 2D map placeholder
- [ ] Draw Master, Slave 1, and Slave 2 labels
- [ ] Refresh screen every 200-500ms
- [ ] Confirm no flickering problem

### 3.3 MAX30102 Test

- [ ] Read raw IR value
- [ ] Detect finger/contact
- [ ] Calculate BPM
- [ ] Reset BPM when finger is removed
- [ ] Print BPM to Serial Monitor

### 3.4 GY-511 / LSM303DLHC Test

- [ ] Wire GY-511 VCC to 3.3V, GND to GND, SDA to GPIO21, and SCL to GPIO22
- [ ] Run `firmware/tests/i2c_scanner_test`
- [ ] Confirm accelerometer address `0x19`
- [ ] Confirm magnetometer address `0x1E`
- [ ] If MAX30102 shares the I2C bus, confirm it appears at `0x57`
- [ ] Run `firmware/tests/gy511_test`
- [ ] Confirm Serial Monitor prints `GY-511 init OK`
- [ ] Tilt the sensor and confirm accelerometer X/Y/Z changes
- [ ] Rotate the sensor and confirm magnetometer X/Y/Z changes
- [ ] Confirm heading output stays in the `0-359 deg` range
- [ ] Repeat for Slave 1 GY-511 module
- [ ] Repeat for Slave 2 GY-511 module
- [ ] Record wiring or heading stability issues before slave firmware integration

### 3.5 UWB Pair Test

- [ ] Connect B&T BU01 DW1000 LDO module to ESP32
- [ ] Confirm power input pin and 3.3V-safe signal levels
- [ ] Test UART/SPI communication
- [ ] Measure 1m, 2m, 5m, 10m
- [ ] Repeat for M-S1, M-S2, and S1-S2 pairs
- [ ] Save results in test-logs/uwb-pair-test.md

Output:

```text
All required modules work individually
```

---

## Phase 4 - Create Communication and Packet Layer

- [ ] Create Packets.h
- [ ] Create slave status packet
- [ ] Create distance packet
- [ ] Create team map packet
- [ ] Send BPM, battery, and ID from slaves to master
- [ ] Receive status on master
- [ ] Add node ID handling
- [ ] Add timeout detection
- [ ] Add support for 2 slave nodes
- [ ] Add master broadcast packet back to all screens

Output:

```text
Master and slaves can exchange structured data
```

---

## Phase 5 - Build UWB Triangle Ranging

- [ ] Measure Master <-> Slave 1 distance
- [ ] Measure Master <-> Slave 2 distance
- [ ] Measure Slave 1 <-> Slave 2 distance
- [ ] Prevent ranging schedule collisions
- [ ] Store latest distance for each pair
- [ ] Add distance quality/state
- [ ] Mark map invalid when any required pair is missing
- [ ] Save triangle test results

Output:

```text
Master has all 3 distances needed for relative 2D mapping
```

---

## Phase 6 - Build Relative 2D Map Solver

- [ ] Put Master at the center/reference point
- [ ] Place Slave 1 on the x-axis using M-S1 distance
- [ ] Compute Slave 2 position using M-S2 and S1-S2 distances
- [ ] Handle impossible triangle values
- [ ] Handle noisy distance values
- [ ] Add basic smoothing
- [ ] Add map valid/invalid state
- [ ] Unit-test solver with known triangles

Output:

```text
Master computes relative 2D coordinates from UWB distances
```

---

## Phase 7 - Build Slave Screen UI

- [ ] Draw relative 2D map
- [ ] Draw Master position
- [ ] Draw Slave 1 position
- [ ] Draw Slave 2 position
- [ ] Show S1 BPM and battery
- [ ] Show S2 BPM and battery
- [ ] Show Master OK/status
- [ ] Show map invalid/ranging lost state
- [ ] Refresh display every 200-500ms

Output:

```text
Every slave screen shows the same team overview
```

---

## Phase 8 - Build Master Firmware

- [ ] Receive slave status packets
- [ ] Collect UWB distance data
- [ ] Compute relative 2D map
- [ ] Print live data through Serial Monitor
- [ ] Broadcast TeamMapPacket to both slaves
- [ ] Add timeout if slave signal is lost
- [ ] Add map invalid state if any distance pair is missing

Output:

```text
Master computes and broadcasts the team map
```

---

## Phase 9 - Power System

- [ ] Connect Li-Po battery
- [ ] Connect TP4056 charger
- [ ] Add power switch
- [ ] Add 3.3V regulator
- [ ] Measure output voltage
- [ ] Add 470uF capacitor near ESP32/UWB
- [ ] Add 10uF and 0.1uF near sensors
- [ ] Test while display is on
- [ ] Test while UWB is active
- [ ] Test while charging
- [ ] Test 30-minute runtime
- [ ] Test 1-hour runtime

Output:

```text
Battery-powered prototype is stable
```

---

## Phase 10 - Tabletop Prototype

- [ ] Build master on perfboard
- [ ] Build slave 1 on perfboard
- [ ] Build slave 2 on perfboard
- [ ] Keep wires short
- [ ] Label wires
- [ ] Photograph wiring
- [ ] Update wiring documentation
- [ ] Run complete 3-node system test

Output:

```text
A portable tabletop 1 master + 2 slave prototype
```

---

## Phase 11 - First Field Test

- [ ] Test at 1m triangle spacing
- [ ] Test at 2m triangle spacing
- [ ] Test at 5m triangle spacing
- [ ] Test at 10m triangle spacing
- [ ] Test outdoors
- [ ] Test with human body blocking signal
- [ ] Test near trees
- [ ] Test near metal
- [ ] Test walking movement
- [ ] Test signal lost behavior
- [ ] Save results in test-logs/field-test.md

Output:

```text
Real-world relative map performance data
```

---

## Phase 12 - Wearable Case

- [ ] Measure board dimensions
- [ ] Measure display dimensions
- [ ] Measure battery dimensions
- [ ] Measure USB-C port position
- [ ] Measure MAX30102 skin contact position
- [ ] Design case version 1
- [ ] Add screen cutout
- [ ] Add USB-C hole
- [ ] Add strap mount
- [ ] Add black foam around MAX30102
- [ ] Test wearing comfort
- [ ] Improve case version 2

Output:

```text
Wearable slave prototype
```

---

## Phase 13 - Demo Preparation

- [ ] Clean map UI
- [ ] Make BPM readable
- [ ] Make battery readable
- [ ] Make map invalid state obvious
- [ ] Prepare 3-minute demo script
- [ ] Prepare architecture diagram
- [ ] Prepare system explanation page

Output:

```text
Ready-to-present relative 2D map prototype
```

---

## Phase 14 - Stability Test

- [ ] Run for 1 hour
- [ ] Check ESP32 restart issue
- [ ] Check battery heat
- [ ] Check sensor freeze
- [ ] Check display freeze
- [ ] Check UWB disconnect
- [ ] Check broadcast stability
- [ ] Move nodes in and out of range
- [ ] Verify reconnect behavior

Output:

```text
Prototype is stable enough for demonstration
```

---

## Phase 15 - Final Documentation

- [ ] Architecture diagram
- [ ] Relative 2D map explanation
- [ ] UWB ranging protocol
- [ ] Hardware block diagram
- [ ] Wiring diagram
- [ ] Pin mapping table
- [ ] BOM with price
- [ ] Firmware flowchart
- [ ] Master firmware explanation
- [ ] Slave firmware explanation
- [ ] Test result table
- [ ] Limitation section
- [ ] Future improvement section

Output:

```text
Complete project documentation
```

---

# 10. Recommended Order of Work

Use this order to reduce risk:

1. Test ESP32 boards
2. Test ST7789 240x240 display
3. Test MAX30102
4. Test BU01 UWB pair ranging
5. Test UWB triangle ranging with 3 nodes
6. Build packet communication
7. Build relative 2D map solver
8. Build slave screen UI
9. Build master broadcast flow
10. Add battery power system
11. Build tabletop prototype
12. Field test
13. Build wearable case
14. Prepare final documentation and demo

---

# 11. Repository Creation Prompt

Use the following prompt with Codex, Cursor, ChatGPT, or another AI coding assistant to create the initial repository.

```text
Create a complete starter repository for a project named "BPMWATCH".

Project description:
BPMWATCH is a low-cost wearable team monitoring prototype using ESP32, BU01/DW1000 UWB ranging, heart-rate sensing, battery monitoring, and a shared relative 2D team map. The system contains 1 master node and 2 slave wearable nodes. The system does not use GPS and does not use fixed anchors. Every node can move.

Architecture:
- Master, Slave 1, and Slave 2 each have ESP32 + BU01 UWB.
- Slave 1 and Slave 2 each have BPM sensor, screen, and battery.
- UWB must measure all 3 pair distances:
  - Master <-> Slave 1
  - Master <-> Slave 2
  - Slave 1 <-> Slave 2
- Master computes a relative 2D triangle map from these distances.
- Master combines the map with BPM and battery status.
- Master broadcasts the result back to every screen.
- Every screen shows the same relative 2D team overview.

Important constraints:
- Do not use GPS.
- Do not require fixed anchors.
- Keep the project low-cost and suitable for parts available in Thailand.
- Use UWB for real distance measurement.
- The MVP requires 1 master + 2 slaves because the relative 2D map needs 3 distance pairs.
- Compass and SOS are future optional features, not MVP blockers.
- Use PlatformIO for ESP32 firmware.
- Keep code modular and easy to test.
- Include clear documentation and TODOs.

Please generate the repository structure below:

bpmwatch/
|-- README.md
|-- LICENSE
|-- .gitignore
|-- docs/
|   |-- architecture.md
|   |-- bom.md
|   |-- pin-map.md
|   |-- relative-2d-map.md
|   |-- ranging-protocol.md
|   |-- test-plan.md
|   |-- wiring-master.md
|   |-- wiring-slave.md
|   `-- limitations.md
|-- firmware/
|   |-- master/
|   |   |-- platformio.ini
|   |   `-- src/
|   |       |-- main.cpp
|   |       |-- Packets.h
|   |       |-- RelativeMapSolver.h
|   |       |-- RelativeMapSolver.cpp
|   |       |-- UwbRanging.h
|   |       |-- UwbRanging.cpp
|   |       |-- TeamBroadcaster.h
|   |       `-- TeamBroadcaster.cpp
|   |-- slave/
|   |   |-- platformio.ini
|   |   `-- src/
|   |       |-- main.cpp
|   |       |-- Packets.h
|   |       |-- DisplayUI.h
|   |       |-- DisplayUI.cpp
|   |       |-- HeartRateSensor.h
|   |       |-- HeartRateSensor.cpp
|   |       |-- BatteryMonitor.h
|   |       |-- BatteryMonitor.cpp
|   |       |-- UwbRanging.h
|   |       `-- UwbRanging.cpp
|   |-- tests/
|   |   |-- esp32_blink/
  |   |   |-- st7789_display_test/
|   |   |-- max30102_test/
|   |   |-- uwb_pair_test/
|   |   |-- uwb_triangle_test/
|   |   `-- broadcast_map_test/
|   `-- legacy/
|-- hardware/
|   |-- diagrams/
|   |-- case/
|   `-- photos/
|-- test-logs/
|   |-- uwb-pair-test.md
|   |-- uwb-triangle-test.md
|   |-- battery-test.md
|   `-- field-test.md
`-- tools/
    `-- mac-address-scanner/

Generate starter content for:

1. README.md
- Project overview
- Relative 2D map concept
- MVP scope
- Hardware overview
- Firmware overview
- Build instructions
- Development phases
- Safety warning about 3.3V UWB module
- Current status section
- TODO section

2. docs/architecture.md
- Explain master/slave architecture
- Explain all 3 UWB distance pairs
- Explain data flow
- Include simple ASCII block diagrams
- Explain why GPS is not used
- Explain why fixed anchors are not used
- Explain that the output is a relative 2D map, not absolute world coordinates

3. docs/relative-2d-map.md
- Explain triangle-based coordinate solving
- Put Master at the reference point
- Place Slave 1 on x-axis
- Compute Slave 2 using M-S2 and S1-S2 distances
- Explain invalid triangle handling
- Explain noisy distance smoothing

4. docs/ranging-protocol.md
- Explain how to schedule UWB ranging for M-S1, M-S2, and S1-S2
- Explain collision avoidance
- Explain distance quality state
- Explain timeout behavior

5. docs/bom.md
- Include hardware tables for master and slaves
- Include recommended specs
- Include quantity for 1 master + 2 slaves
- Include budget notes

6. docs/pin-map.md
- Include pin mapping tables for master and slave
- Leave TODO markers for pins that must be confirmed on real hardware

7. docs/test-plan.md
- Include module test checklist
- Include UWB pair test checklist
- Include UWB triangle test checklist
- Include map display checklist
- Include battery test checklist
- Include field test checklist

8. docs/wiring-master.md
- Include master wiring notes
- Include power wiring notes
- Include UWB wiring warning

9. docs/wiring-slave.md
- Include slave wiring notes
- Include ST7789 240x240 display, MAX30102, battery, and UWB wiring notes

10. docs/limitations.md
- Mention that this is not a true absolute 2D map
- Mention that there is no GPS location
- Mention that fixed anchors are not used
- Mention that UWB antenna orientation and obstruction affect accuracy
- Mention that BPM sensor is not medical grade

11. firmware/master
- Create a PlatformIO ESP32 project
- main.cpp should initialize Serial, communication, UWB ranging, relative map solver, and team map broadcast
- Use clean modular code
- Add TODO comments where BU01 protocol details are needed

12. firmware/slave
- Create a PlatformIO ESP32 project
- main.cpp should initialize Serial, display, heart-rate sensor, battery monitor, UWB ranging, and communication
- Send SlaveStatusPacket to the master every 200-500 ms
- Receive TeamMapPacket from master
- Draw a simple relative 2D map placeholder
- Add TODO comments for sensor calibration and real UWB integration

13. Shared Packets.h
- Define SlaveStatusPacket, DistancePacket, and TeamMapPacket.

14. RelativeMapSolver
- Provide code that computes relative 2D coordinates from 3 distances.
- Handle invalid triangle measurements safely.
- Use placeholder smoothing if needed.

15. UwbRanging
- Provide placeholder functions for BU01/DW1000 ranging.
- Include TODO comments for exact UART/SPI protocol implementation.

16. Test projects
- Add minimal test sketches or README files for:
  - ESP32 blink
  - ST7789 240x240 display
  - MAX30102
  - UWB pair ranging
  - UWB triangle ranging
  - Broadcast map packet

17. tools/mac-address-scanner
- Add a simple PlatformIO or Arduino sketch that prints the ESP32 MAC address over Serial.

Coding style:
- Use C++ for ESP32 firmware
- Use clear function names
- Avoid overly complex architecture
- Add comments that explain hardware assumptions
- Use TODO comments for hardware-specific values
- Make the code compile as much as possible without requiring all hardware modules immediately
- Prefer placeholder classes over incomplete broken code

Output:
- Generate all files and folders
- Provide a short explanation of how to open the project in VS Code with PlatformIO
- Provide the first 5 commands or actions the developer should run
```

---

# 12. First 5 Developer Actions

After creating the repository:

```bash
git init
git add .
git commit -m "Initial BPMWATCH relative 2D map project structure"
```

Then:

1. Open the repository in VS Code
2. Install PlatformIO extension
3. Connect one ESP32
4. Open `firmware/tests/esp32_blink`
5. Upload and confirm Serial Monitor works

---

# 13. Suggested First Milestone

## Milestone 1 - Hardware Module Validation

Target result:

```text
Every required hardware module works alone before integration.
```

Checklist:

- [ ] ESP32 flash success
- [ ] MAC address printed for all 3 boards
- [ ] ST7789 240x240 screen works
- [ ] MAX30102 raw IR value works
- [ ] BU01 UWB pair communication works
- [ ] Battery power is stable

Do not start the full relative map firmware until this milestone is complete.

---

# 14. Suggested Second Milestone

## Milestone 2 - UWB Triangle Ranging

Target result:

```text
All 3 UWB distance pairs work.
```

Checklist:

- [ ] Master <-> Slave 1 distance works
- [ ] Master <-> Slave 2 distance works
- [ ] Slave 1 <-> Slave 2 distance works
- [ ] Ranging schedule avoids collisions
- [ ] Distances are logged
- [ ] Missing distance makes map invalid

---

# 15. Suggested Third Milestone

## Milestone 3 - Shared Relative 2D Team Map

Target result:

```text
Master computes the relative 2D map and every screen shows the same team overview.
```

Checklist:

- [ ] Slave 1 uses node ID 1
- [ ] Slave 2 uses node ID 2
- [ ] Master computes relative coordinates
- [ ] Master broadcasts TeamMapPacket
- [ ] Slave 1 screen shows the team map
- [ ] Slave 2 screen shows the team map
- [ ] BPM and battery values are visible
- [ ] Signal/ranging lost state is visible

---

# 16. Future Improvements

Possible improvements after the MVP:

- Add SOS button and alert states
- Add vibration alert patterns
- Add compass or IMU heading display
- Add UWB angle-of-arrival hardware if budget allows
- Add better battery percentage estimation
- Add BLE phone app
- Add data logging
- Add waterproof case
- Add mesh communication
- Add more slave nodes
- Add emergency event history
- Add better map smoothing
