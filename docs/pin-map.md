# Two-Node Radar Pin Map

Node A and Node B use the same signal wiring. Only the ESP32 STA MAC and fixed DW1000 role differ.

## Shared Pin Contract

| Function | ESP32 pin | Bus/notes |
|---|---:|---|
| BU01 SPI SCK | GPIO18 | Default global `SPI` |
| BU01 SPI MISO | GPIO19 | Default global `SPI` |
| BU01 SPI MOSI | GPIO23 | Default global `SPI` |
| BU01 SPI CS/SS | GPIO5 | DW1000 chip select |
| BU01 IRQ | GPIO34 | Input-only interrupt pin |
| BU01 RST | GPIO4 | DW1000 reset |
| ST7789 SCL/SCK | GPIO14 | Dedicated `HSPI` / `SPIClass` |
| ST7789 SDA/MOSI | GPIO13 | Dedicated `HSPI` / `SPIClass` |
| ST7789 BLC | GPIO25 | Backlight control |
| ST7789 DC | GPIO26 | Display data/command |
| ST7789 RES | GPIO27 | Display reset |
| GY-511 SDA | GPIO21 | Shared I2C data |
| GY-511 SCL | GPIO22 | Shared I2C clock |
| MAX30102 SDA | GPIO21 | Shared I2C data |
| MAX30102 SCL | GPIO22 | Shared I2C clock |
| Battery ADC | Not assigned | Add a 3.3V-safe resistor divider before selection |
| Optional button | Not assigned | Use a pull-up/pull-down and debounce when added |

## Node Identity

| Product node | Node ID | ESP32 STA MAC | Fixed DW1000 role |
|---|---:|---|---|
| Node A | 0 | `0C:8A:D3:7C:E5:A4` | Anchor |
| Node B | 1 | `1C:75:C4:F4:E9:D4` | Tag |

The old `FC:FA:31:FE:8C:E0` board is outside the current two-node product and may be retained as a spare.

## Bus Rules

- The vendored DW1000 library owns default global `SPI`.
- The no-CS ST7789 must stay on dedicated HSPI because it cannot be deselected on a shared bus.
- The confirmed ZJY-IPS130-V2.0 initialization uses a manual RES pulse, `SPI_MODE3`, and 8 MHz transactions set through `tft.setSPISpeed(8000000)` after `init()`.
- GY-511 uses `0x19` and `0x1E`; MAX30102 uses `0x57`, so they can share I2C GPIO21/22.
- All module signal levels must remain 3.3V-compatible.
