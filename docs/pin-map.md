# Pin Map

Pin choices must be confirmed on real ESP32 boards and actual BU01 module mode.

## Master

| Function | ESP32 Pin | Notes |
|---|---:|---|
| BU01 TX/RX or SPI | TODO | Confirm UART/SPI mode |
| BU01 IRQ/RST | TODO | If required by module |
| Battery ADC | TODO | Use divider safe for 3.3V ADC |
| Optional button | TODO | Reset ranging/calibration |

## Slave

| Function | ESP32 Pin | Notes |
|---|---:|---|
| BU01 TX/RX or SPI | TODO | Confirm UART/SPI mode |
| ST7789 SCK | TODO | SPI clock |
| ST7789 MOSI | TODO | SPI data |
| ST7789 CS | TODO | Display chip select |
| ST7789 DC | TODO | Display data/command |
| ST7789 RST | TODO | Display reset |
| MAX30102 SDA | TODO | I2C data |
| MAX30102 SCL | TODO | I2C clock |
| Battery ADC | TODO | Use divider safe for 3.3V ADC |

## Node IDs

| Node | ID | MAC Address |
|---|---:|---|
| Master | 0 | TODO |
| Slave 1 | 1 | TODO |
| Slave 2 | 2 | TODO |
