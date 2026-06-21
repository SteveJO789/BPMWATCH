# Legacy Relative 2D Map

> Superseded on 2026-06-21. This document describes the former three-node triangle architecture and is retained only for project history.

The map solver uses 3 measured distances:

- `dM1`: Master to Slave 1
- `dM2`: Master to Slave 2
- `d12`: Slave 1 to Slave 2

## Coordinate Setup

Use a simple reference frame:

```text
Master = (0, 0)
Slave 1 = (dM1, 0)
Slave 2 = (x, y)
```

Compute Slave 2:

```text
x = (dM2^2 + dM1^2 - d12^2) / (2 * dM1)
y = sqrt(dM2^2 - x^2)
```

## Invalid Triangle Handling

The map is invalid when:

- Any distance is zero or negative.
- `dM1` is too small to place Slave 1.
- Triangle inequality fails.
- The square root term becomes negative after noise tolerance.

When invalid, the firmware should keep the last known good map briefly, then show a ranging lost state.

## Noise Smoothing

Start with simple exponential smoothing:

```text
smoothed = previous * 0.7 + measured * 0.3
```

Do not over-smooth during early testing because bad UWB measurements must remain visible.
