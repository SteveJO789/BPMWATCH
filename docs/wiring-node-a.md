# Node A Wiring

Node A is an equal Radar Node with the fixed DW1000 Anchor radio role. Its ESP32 STA MAC is `0C:8A:D3:7C:E5:A4`.

## Modules

Node A must contain ESP32, BU01/DW1000, no-CS ST7789, GY-511, MAX30102, and the protected battery power chain. This adds the display and sensors that were absent from the former Master hardware definition.

## Signal Wiring

| Module signal | ESP32 pin |
|---|---:|
| BU01 SCK / MISO / MOSI | GPIO18 / GPIO19 / GPIO23 |
| BU01 CS / IRQ / RST | GPIO5 / GPIO34 / GPIO4 |
| ST7789 SCL / SDA | GPIO14 / GPIO13 |
| ST7789 BLC / DC / RES | GPIO25 / GPIO26 / GPIO27 |
| GY-511 SDA / SCL | GPIO21 / GPIO22 |
| MAX30102 SDA / SCL | GPIO21 / GPIO22 |

The ST7789 labels `SCL` and `SDA` mean SPI SCK and MOSI on this display. They are not I2C.

## Power

```text
602030 Li-Po -> protected TP4056 -> power switch -> TPS63802 3.3V
                                                        |
                         ESP32 + BU01 + ST7789 + GY-511 + MAX30102
```

- Share ground across every module.
- Add bulk capacitance at the regulator output.
- Add local 10uF and 0.1uF decoupling near active modules.
- Keep UWB and ESP32 antenna areas clear of battery, display cable, and metal.
- Confirm the exact BU01 breakout power-input label; do not infer 5V tolerance from the `LDO` name.

## Mounting

Mount GY-511 flat and record its X/Y direction relative to the display top. Firmware axis mapping will be validated on the bench before the enclosure orientation is fixed.
