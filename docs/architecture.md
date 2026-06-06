# Architecture

BPMWATCH uses 1 master node and 2 slave wearable nodes.

```text
        UWB M-S1                UWB M-S2
Slave 1 <--------> Master <--------> Slave 2
   ^                                      ^
   |------------ UWB S1-S2 --------------|
```

Each slave sends BPM, battery, signal state, and node ID to the master. The master collects all 3 UWB distance pairs, computes a relative 2D map, and broadcasts the same team map back to every screen.

## Data Flow

```text
Slave 1 status ----\
Slave 2 status -----+--> Master --> RelativeMapSolver --> TeamMapPacket
UWB distances ------/                                \--> all screens
```

## Why No GPS

GPS does not work reliably indoors, under dense trees, or in many tactical/exploration conditions. It also adds cost and power use. This project only needs team-relative awareness for the MVP.

## Why No Fixed Anchors

Fixed anchors make setup harder in outdoor or moving-team scenarios. The SVG architecture assumes every node can move, so the map is generated from distances inside the team.

## Output Meaning

The output is not absolute world coordinates. It is a relative triangle:

- Master is the reference point.
- Slave 1 is placed on the x-axis.
- Slave 2 is computed from the measured distances.
