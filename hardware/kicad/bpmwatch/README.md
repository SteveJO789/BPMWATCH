# BPMWATCH KiCad Schematic Status

The KiCad files in this folder predate the 2026-06-21 two-node Radar pivot. They describe an unequal Master/repeated-Slave concept and are not the current wiring source of truth.

Use [`docs/pin-map.md`](../../../docs/pin-map.md), [`docs/wiring-node-a.md`](../../../docs/wiring-node-a.md), and [`docs/wiring-node-b.md`](../../../docs/wiring-node-b.md) for current bench wiring.

## Required Schematic Revision

The next schematic revision must contain two equivalent Radar Nodes:

- ESP32-WROOM-32
- BU01/DW1000 on default SPI
- No-CS ST7789 on dedicated HSPI
- GY-511 and MAX30102 on shared I2C GPIO21/22
- Protected Li-Po charging and TPS63802 3.3V rail

Only the node identity and fixed DW1000 Anchor/Tag role differ.

## Existing Artifact Use

The existing schematic and exports remain historical module-level interconnect references. They are not production PCB signoff, and their Master/Slave labels must not be copied into new designs.
