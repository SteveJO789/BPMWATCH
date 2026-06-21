# Legacy Slave Wiring

> Superseded on 2026-06-21. Current wiring is documented in `docs/wiring-node-a.md` and `docs/wiring-node-b.md`.

Each slave uses ESP32-WROOM-32, B&T BU01 DW1000 LDO UWB breakout, an IPS TFT LCD 240x240 ST7789 display, GY-511/LSM303DLHC motion/compass sensor, MAX30102 BPM sensor, SOS button, and battery power.

## Power

```text
602030 Li-Po 3.7V -> TP4056 Type-C with protection -> power switch -> TPS63802 buck-boost -> 3.3V rail
```

All modules should share common ground. Add power-filter capacitors near the buck-boost output and local decoupling near the ESP32, BU01, display, and sensors.

## ST7789 240x240 Display

Use the display module pins `SCL`, `SDA`, `BLC`, `DC`, and `RES`. On this ST7789 module, `SCL` is SPI `SCK` and `SDA` is SPI `MOSI`; they are not I2C pins. `BLC` should use a PWM-capable GPIO if brightness control is needed. `DC` is the data/command pin and `RES` is the reset pin. The module has no `CS` pin, so firmware uses no-CS ST7789 mode; in Wokwi, the simulated `CS` pin is tied to ground.

## MAX30102

Use I2C wiring. Confirm pullups and module voltage compatibility before long testing.

## GY-511 / LSM303DLHC

Use I2C wiring. It can share the I2C bus with MAX30102 if the breakout addresses and pullups are compatible. It does not share the ST7789 display bus because the display uses SPI.

## SOS Button

Wire as a momentary digital input with a pullup or pulldown resistor. Debounce in firmware and keep it reachable from the enclosure.

## BU01 UWB

Use 3.3V-safe signal levels. Confirm the exact power input pin and UART/SPI mode on the B&T BU01 DW1000 LDO breakout before final wiring. Keep the antenna area clear of metal, battery, and dense wiring when possible.

## Battery Monitor

Use a resistor divider into an ESP32 ADC pin. Do not feed raw Li-Po voltage into an ADC pin.
