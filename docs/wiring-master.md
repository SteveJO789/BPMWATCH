# Master Wiring

The master uses ESP32 + BU01/DW1000 UWB and battery power.

## Power

```text
Li-Po 3.7V -> TP4056 -> power switch -> 3.3V regulator -> ESP32 + BU01
```

Add:

- 470uF capacitor near ESP32/UWB power input.
- 10uF and 0.1uF decoupling near modules.

## UWB Warning

BU01/DW1000 must use 3.3V only. Do not connect 5V to power or signal pins.

## Signal Wiring

Confirm whether the BU01 module is configured for UART or SPI before final wiring. Keep wires short and document actual pins in `docs/pin-map.md`.
