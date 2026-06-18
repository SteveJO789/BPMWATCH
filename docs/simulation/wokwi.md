# Wokwi Simulation

The Wokwi simulation is for firmware behavior before real hardware is ready.

It simulates:

- ESP32 master firmware loop
- Adjustable fake BU01 UWB distances for `M-S1`, `M-S2`, and `S1-S2`
- Adjustable fake MAX30102 BPM values
- Fake battery values
- Adjustable fake GY-511 heading value
- Relative 2D map solving through the real `RelativeMapSolver`
- Color TFT rendering through Wokwi's ILI9341 display part

It does not simulate real BU01 UWB radio physics.

## Display Target

The current hardware target is an IPS TFT LCD 240x240 module with an ST7789 controller. The module labels are:

- `SCL`: SPI clock / SCK
- `SDA`: SPI data / MOSI
- `BLC`: backlight control
- `DC`: data/command
- `RES`: reset

Wokwi does not provide a built-in `wokwi-st7789` part, so the simulation uses `wokwi-ili9341` as a visual surrogate. This lets the map appear on a screen in Wokwi. The real hardware test remains ST7789 no-CS.

## Run

```bash
cd firmware/sim/wokwi
pio run
```

Open the folder in Wokwi or use the Wokwi extension with `wokwi.toml`. The Serial Monitor should print one map frame per second.

## Adaptive Inputs

The diagram has six knob potentiometers:

| Control | Simulates | Range |
|---|---|---:|
| `UWB dM1` | Master to Slave 1 distance | 0.5-6.0 m |
| `UWB dM2` | Master to Slave 2 distance | 0.5-6.0 m |
| `UWB d12` | Slave 1 to Slave 2 distance | 0.5-6.0 m |
| `MAX30102 S1 BPM` | Slave 1 heart rate | 45-160 BPM |
| `MAX30102 S2 BPM` | Slave 2 heart rate | 45-160 BPM |
| `GY-511 heading` | Heading angle | 0-359 deg |

Move the UWB knobs into an impossible triangle to test `mapValid=0`.

Expected output shape:

```text
BPMWATCH Wokwi simulation
Display hardware target:
- Real hardware: IPS TFT LCD 240x240 with ST7789 controller
- Wokwi visual surrogate: ILI9341 SPI TFT
- Real module labels: SCL=SPI SCK, SDA=SPI MOSI, BLC=backlight, DC=data/command, RES=reset
mapValid=1 M=(0.00,0.00) S1=(2.45,0.00) S2=(...) BPM=.../... BAT=... heading=...
```
