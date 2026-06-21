# Node B Wiring

Node B is an equal Radar Node with the fixed DW1000 Tag radio role. Its ESP32 STA MAC is `1C:75:C4:F4:E9:D4`.

## Modules

Node B contains ESP32, BU01/DW1000, no-CS ZJY-IPS130-V2.0 ST7789, GY-511, MAX30102, and the protected battery power chain.

## Signal Wiring

| Module signal | ESP32 pin |
|---|---:|
| BU01 SCK / MISO / MOSI | GPIO18 / GPIO19 / GPIO23 |
| BU01 CS / IRQ / RST | GPIO5 / GPIO34 / GPIO4 |
| ST7789 SCL / SDA | GPIO14 / GPIO13 |
| ST7789 BLC / DC / RES | GPIO25 / GPIO26 / GPIO27 |
| GY-511 SDA / SCL | GPIO21 / GPIO22 |
| MAX30102 SDA / SCL | GPIO21 / GPIO22 |

The no-CS ST7789 uses dedicated HSPI. Initialize the tested ZJY-IPS130-V2.0 with a manual RES pulse, `SPI_MODE3`, and 8 MHz transactions. GY-511 and MAX30102 share I2C GPIO21/22.

## Power

```text
602030 Li-Po -> protected TP4056 -> power switch -> TPS63802 3.3V
                                                        |
                         ESP32 + BU01 + ST7789 + GY-511 + MAX30102
```

- Share ground across every module.
- Add bulk and local decoupling capacitors.
- Verify TPS63802 output during concurrent UWB and ESP-NOW traffic.
- Keep antenna regions away from metal and the battery.

## Mounting

Use the same GY-511-to-display orientation convention as Node A. If physical assembly differs, select the correct compile-time X/Y swap and sign mapping for Node B.
