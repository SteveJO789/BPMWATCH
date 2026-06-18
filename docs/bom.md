# Bill of Materials

## Master Node

| Component | Recommended Spec | Qty |
|---|---:|---:|
| ESP32-WROOM-32 | 3.3V logic, Wi-Fi, 4MB flash or more | 1 |
| B&T BU01 DW1000 LDO UWB Breakout | Confirm UART/SPI mode, power input pin, and 3.3V-safe logic | 1 |
| 602030 Li-Po Battery | 3.7V cell, capacity/discharge current TBD | 1 |
| TP4056 Type-C Charger | With battery protection | 1 |
| TPS63802 Buck-Boost Module | Stable 3.3V rail for ESP32 and UWB load spikes | 1 |
| Power Switch | Mini slide switch or rated power switch | 1 |
| Power-Filter Capacitors | Bulk + local decoupling, values to confirm on bench | 1 set |
| Case | ABS box or 3D printed case | 1 |

## Slave Nodes

| Component | Recommended Spec | Qty for 2 slaves |
|---|---:|---:|
| ESP32-WROOM-32 | 3.3V logic, Wi-Fi, 4MB flash or more | 2 |
| B&T BU01 DW1000 LDO UWB Breakout | Confirm UART/SPI mode, power input pin, and 3.3V-safe logic | 2 |
| IPS TFT LCD 240x240 ST7789 Display | Module labels SCL/SCK, SDA/MOSI, BLC, DC, and RES | 2 |
| GY-511 / LSM303DLHC | I2C accelerometer/compass module | 2 |
| MAX30102 Heart Rate Sensor | I2C heart-rate module | 2 |
| SOS Button | Momentary push button | 2 |
| 602030 Li-Po Battery | 3.7V cell, capacity/discharge current TBD | 2 |
| TP4056 Type-C Charger | With battery protection | 2 |
| TPS63802 Buck-Boost Module | Stable 3.3V rail for ESP32, UWB, display, and sensors | 2 |
| Power Switch | Mini slide switch or rated power switch | 2 |
| Power-Filter Capacitors | Bulk + local decoupling, values to confirm on bench | 2 sets |
| Watch Strap | Velcro tactical strap | 2 |
| Case | 3D printed or small ABS case | 2 |

## Budget Notes

Prioritize the B&T BU01 DW1000 LDO UWB modules, stable TPS63802 3.3V power rail, display readability, MAX30102 signal quality, GY-511 bus compatibility, and safe Li-Po charging first. Vibration and waterproofing can wait until the relative map MVP works.
