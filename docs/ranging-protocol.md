# Ranging Protocol

The MVP must measure 3 UWB pairs:

- Master <-> Slave 1
- Master <-> Slave 2
- Slave 1 <-> Slave 2

## Suggested Schedule

```text
Slot 1: Master ranges Slave 1
Slot 2: Master ranges Slave 2
Slot 3: Slave 1 ranges Slave 2
Slot 4: Master collects latest distance packets
Slot 5: Master broadcasts TeamMapPacket
```

The exact B&T BU01 DW1000 LDO UWB breakout protocol is still TODO. Keep the schedule deterministic so multiple nodes do not range at the same time, and confirm whether the purchased breakout exposes UART mode or requires direct SPI control.

## Collision Avoidance

- Use fixed node IDs: Master = 0, Slave 1 = 1, Slave 2 = 2.
- Only one ranging pair should be active per slot.
- If a slot times out, mark that pair stale and continue.

## Distance Quality

Use a simple quality byte at first:

- `0`: invalid or missing
- `1`: weak/stale
- `2`: valid
- `3`: strong/recent

## Timeout Behavior

If any required distance pair is stale, `TeamMapPacket.mapValid` must be `0`.
