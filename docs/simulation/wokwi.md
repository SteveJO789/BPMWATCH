# Wokwi Simulation Status

The checked-in `firmware/sim/wokwi` project predates the two-node Radar pivot. It demonstrates the superseded three-node triangle UI with mocked values and is not validation of the current Radar MVP.

## Current Use

The old simulation may still be used to inspect:

- ESP32 display rendering through Wokwi's ILI9341 visual surrogate
- Basic SPI display behavior
- Historical triangle-map UI behavior

Do not use it to validate current Node A/B data flow, movement-bearing estimation, ESP-NOW, reciprocal bearing, or auto-range behavior.

## Current Hardware Display

Real hardware uses a no-CS ST7789 240x240 display:

- Module `SCL` is SPI SCK.
- Module `SDA` is SPI MOSI.
- `BLC` controls the backlight.
- `DC` selects command/data.
- `RES` resets the display.
- The display runs on dedicated HSPI and does not share the DW1000 default SPI bus.

Wokwi does not provide a built-in ST7789 part matching this module, so the ILI9341 may remain a visual surrogate in a future Radar simulation.

## Replacement Simulation Requirements

A current simulation must model one Radar Node screen with controllable inputs for:

- UWB distance
- Compass heading
- Forward/right acceleration
- Peer BPM validity and value
- UWB and ESP-NOW link states

It must show the north-up radar, movement-bearing response, automatic `5/10/20/40m` scale, retained UWB-lost dot, and stale Peer BPM behavior.
