# Slave Wiring

Each slave uses ESP32, BU01/DW1000 UWB, ST7789 display, MAX30102 BPM sensor, and battery power.

## Power

```text
Li-Po 3.7V -> TP4056 -> power switch -> 3.3V regulator
```

All modules should share common ground.

## ST7789

Use SPI wiring. Confirm actual pins in `docs/pin-map.md`.

## MAX30102

Use I2C wiring. Confirm pullups and module voltage compatibility before long testing.

## BU01/DW1000

Use 3.3V only. Keep the antenna area clear of metal, battery, and dense wiring when possible.

## Battery Monitor

Use a resistor divider into an ESP32 ADC pin. Do not feed raw Li-Po voltage into an ADC pin.
