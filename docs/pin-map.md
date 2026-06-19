# Pin Map

Pin choices must be confirmed on real ESP32 boards and the actual B&T BU01 DW1000 LDO module mode.

## Master

| Function | ESP32 Pin | Notes |
|---|---:|---|
| B&T BU01 SPI SCK | GPIO18 | Default global `SPI`, shared with thotro DW1000 library |
| B&T BU01 SPI MISO | GPIO19 | Default global `SPI` |
| B&T BU01 SPI MOSI | GPIO23 | Default global `SPI` |
| B&T BU01 SPI CS | GPIO5 | DW1000 chip select |
| B&T BU01 IRQ | GPIO34 | Input-only interrupt pin |
| B&T BU01 RST | GPIO4 | DW1000 reset |
| Battery ADC | TODO | Use divider safe for 3.3V ADC |
| Power switch | N/A | In battery input path before buck-boost module |
| Optional button | TODO | Reset ranging/calibration |

## Slave

| Function | ESP32 Pin | Notes |
|---|---:|---|
| B&T BU01 SPI SCK | GPIO18 | Default global `SPI`, shared with thotro DW1000 library |
| B&T BU01 SPI MISO | GPIO19 | Default global `SPI` |
| B&T BU01 SPI MOSI | GPIO23 | Default global `SPI` |
| B&T BU01 SPI CS | GPIO5 | DW1000 chip select |
| B&T BU01 IRQ | GPIO34 | Input-only interrupt pin |
| B&T BU01 RST | GPIO4 | DW1000 reset |
| ST7789 SCL | GPIO14 | Separate `HSPI` / `SPIClass`, SPI clock / SCK, not I2C |
| ST7789 SDA | GPIO13 | Separate `HSPI` / `SPIClass`, SPI MOSI, not I2C |
| ST7789 BLC | GPIO25 | Backlight control, PWM-capable pin |
| ST7789 DC | GPIO26 | Display data/command |
| ST7789 RES | GPIO27 | Display reset |
| GY-511 / LSM303DLHC SDA | TODO | I2C data, can share bus with MAX30102 if addresses do not conflict |
| GY-511 / LSM303DLHC SCL | TODO | I2C clock |
| MAX30102 SDA | TODO | I2C data |
| MAX30102 SCL | TODO | I2C clock |
| SOS button | TODO | Use pullup or pulldown and debounce in firmware |
| Battery ADC | TODO | Use divider safe for 3.3V ADC |
| Power switch | N/A | In battery input path before buck-boost module |

## Node IDs

| Node | ID | MAC Address |
|---|---:|---|
| Master | 0 | `0C:8A:D3:7C:E5:A4` |
| Slave 1 | 1 | `1C:75:C4:F4:E9:D4` |
| Slave 2 | 2 | `FC:FA:31:FE:8C:E0` |
