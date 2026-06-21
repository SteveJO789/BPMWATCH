# Two-Node Radar Footprint Research

Research baseline: 2026-06-09. Architecture updated: 2026-06-21.

The current target has two equivalent Radar Nodes. Exact purchased breakouts must be measured before assigning production footprints.

| Module per node | Qty total | Footprint direction | Status |
|---|---:|---|---|
| ESP32-WROOM-32 | 2 | `RF_Module:ESP32-WROOM-32` only for a bare castellated module; use the exact dev-board headers otherwise | Confirm board variant |
| BU01 UWB breakout | 2 | Custom footprint from the exact BU01 package drawing | Confirm pad pattern and antenna keepout |
| ZJY-IPS130-V2.0 no-CS ST7789 240x240 | 2 | Measure the purchased header count, board outline, and mounting holes | Existing schematic symbol may not match |
| MAX30102 breakout | 2 | Custom breakout/header footprint from measured board | Confirm row spacing and orientation |
| GY-511 / LSM303DLHC | 2 | Header plus measured board outline | Confirm header direction and axis marking |
| Protected TP4056 Type-C module | 2 | Custom module footprint matching exact IN/BAT/OUT pads | Board variants differ |
| TPS63802 module | 2 | Custom breakout footprint; do not substitute bare-IC layout without a regulator redesign | Vendor/module not yet identified |
| 602030 Li-Po connector | 2 | Exact connector, JST-PH only if confirmed | Battery body is a mechanical keepout |
| Power switch | 2 | Exact purchased switch footprint | Confirm pitch and actuator direction |
| Battery divider resistors | 4 | 0805 recommended for hand assembly | Values selected during ADC design |
| Bulk/local capacitors | Per rail | 0805/1206 according to capacitance and voltage | Validate under radio bursts |

## Sources Previously Checked

- KiCad official `RF_Module`, connector, switch, resistor, and capacitor libraries
- Ai-Thinker BU01 public dimensions and SPI interface information
- ST7789 240x240 module references
- MAX30102 breakout references
- GY-511 / LSM303DLHC module references
- TP4056 protection module references
- Texas Instruments TPS63802 documentation

## Blockers Before PCB Layout

1. Regenerate the schematic as two equivalent Radar Nodes instead of the pre-pivot Master/Slave drawing.
2. Confirm every purchased breakout's dimensions, header count, and pin order.
3. Decide whether the PCB carries bare modules or sockets for development breakouts.
4. Add ESP32 and BU01 antenna keepouts.
5. Add battery, display, sensor-contact, and enclosure mechanical constraints.
6. Validate power integrity during simultaneous ESP-NOW, UWB, display, and sensor activity.
