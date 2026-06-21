# Two-Node Radar Architecture

BPMWATCH contains two equal wearable Radar Nodes. Each node displays itself at the center and the other node as its Peer.

```text
Node A / DW1000 Anchor  <--- calibrated UWB distance --->  Node B / DW1000 Tag
          |                                                       |
          +<------ ESP-NOW: Peer BPM + bearing estimate -------->+
```

Anchor and Tag are fixed DW1000 radio roles. They do not create an application-level Master/Slave hierarchy.

## Per-Node Data Flow

```text
DW1000 distance ------\
GY-511 heading --------+--> Radar state --> north-up ST7789 display
GY-511 acceleration --/

Local MAX30102 BPM --> ESP-NOW --> Peer display
Accepted bearing ----> ESP-NOW --> reciprocal angle on Peer display
```

## UWB Responsibility

UWB provides distance only. Production firmware retains the proven Anchor/Tag ranging flow, antenna delay `16555`, five-sample median filter, and recovery behavior.

UWB does not provide direction. The displayed bearing is a visual estimate derived from current-node movement and heading.

## Movement Bearing

The node uses mapped GY-511 acceleration axes to estimate a short-lived horizontal acceleration direction, then rotates that direction into a north-up compass frame.

When cumulative UWB distance changes by more than `0.25m` during accepted movement:

- Decreasing range places the Peer along the movement bearing.
- Increasing range places the Peer approximately 180 degrees behind it.
- Stable range keeps the previous Peer bearing.

If one node accepts a bearing, ESP-NOW sends it to the Peer. The Peer applies `+180 degrees` so both displays update when only one node moves.

## ESP-NOW Responsibility

ESP-NOW carries small bidirectional status packets:

- Peer BPM and validity
- Accepted bearing and validity
- Node ID, packet version, and sequence

It uses Wi-Fi station mode without creating an AP or browser UI. ESP-NOW freshness and UWB freshness are tracked separately.

## Display Meaning

- North is always at the top.
- The current node is always at the center.
- The Peer radius represents filtered UWB distance within the active `5/10/20/40m` band.
- The Peer angle is an estimate, not a measured UWB angle or geographic position.
- A gray outline dot means the last UWB position is being retained after link loss.

## Historical Architecture

The former three-node triangle solver and `TeamMapPacket` flow are superseded. Historical documents are under [`legacy/three-node`](legacy/three-node/README.md).
