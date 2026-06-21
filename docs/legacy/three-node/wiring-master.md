# Legacy Master Wiring

> Superseded on 2026-06-21. Current wiring is documented in `docs/wiring-node-a.md` and `docs/wiring-node-b.md`.

The master uses ESP32-WROOM-32 + B&T BU01 DW1000 LDO UWB breakout and battery power.

## Power

```text
602030 Li-Po 3.7V -> TP4056 Type-C with protection -> power switch -> TPS63802 buck-boost -> 3.3V rail -> ESP32 + BU01
```

Add:

- Bulk capacitor near the TPS63802 output and ESP32/UWB power input.
- Local 10uF and 0.1uF decoupling near modules.
- Shared ground between charger output, buck-boost module, ESP32, and BU01.

## UWB Warning

B&T BU01 DW1000 LDO must use 3.3V-compatible signal pins. Confirm the exact power input pin before wiring; the LDO label does not automatically make every pin 5V tolerant.

## Signal Wiring

Confirm whether the B&T BU01 DW1000 LDO breakout is configured for UART or SPI before final wiring. Keep wires short and document actual pins in `docs/pin-map.md`.
