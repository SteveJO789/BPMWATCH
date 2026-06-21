# BPMWATCH Radar Context

BPMWATCH is a two-node wearable radar system. Each node shows itself at the center of a north-up display and estimates the direction and distance of the other node.

## Language

**Radar Node**:
One of the two equal wearable devices in the product. Each Radar Node has UWB ranging, motion and heading sensing, heart-rate sensing, and a display.
_Avoid_: Master, Slave, tracker

**Node A**:
The Radar Node using ESP32 STA MAC `0C:8A:D3:7C:E5:A4`. It has the fixed DW1000 Anchor radio role, but it is not an application-level master.
_Avoid_: Master Node

**Node B**:
The Radar Node using ESP32 STA MAC `1C:75:C4:F4:E9:D4`. It has the fixed DW1000 Tag radio role, but it is not an application-level slave.
_Avoid_: Slave 1

**Peer**:
The other Radar Node from the current node's point of view.
_Avoid_: Remote slave, tracked node

**Radar Bearing**:
The estimated north-up angle from the current node to its Peer. It is a visual movement-based estimate, not an angle measured by the DW1000.
_Avoid_: UWB angle, true bearing

**Peer BPM**:
The heart-rate value measured by the Peer and received over the peer data link.
_Avoid_: Local BPM

**Legacy Three-Node Map**:
The superseded architecture that used one master, two slaves, three UWB distances, and a shared triangle map.
_Avoid_: Current architecture
