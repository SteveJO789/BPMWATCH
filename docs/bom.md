# Two-Node Radar Bill of Materials

Node A and Node B use the same hardware. Quantities below are for the complete two-node MVP.

| Component | Requirement | Qty |
|---|---|---:|
| ESP32-WROOM-32 board | 3.3V logic, Wi-Fi/ESP-NOW, 4MB flash or more | 2 |
| B&T BU01 DW1000 LDO breakout | SPI interface, confirmed power-input pin, 3.3V-safe signals | 2 |
| IPS TFT LCD 240x240 ST7789 | No-CS module exposing SCL/SCK, SDA/MOSI, BLC, DC, RES | 2 |
| GY-511 / LSM303DLHC | I2C accelerometer and compass, addresses `0x19` and `0x1E` | 2 |
| MAX30102 breakout | I2C heart-rate sensor, address `0x57` | 2 |
| 602030 Li-Po battery | 3.7V cell; verify actual capacity and discharge rating | 2 |
| TP4056 Type-C charger | Include battery protection | 2 |
| TPS63802 buck-boost module | Stable 3.3V rail for ESP32, UWB, display, and sensors | 2 |
| Power switch | Rated for the battery path | 2 |
| Bulk and local capacitors | Bulk rail support plus 10uF/0.1uF near active modules | 2 sets |
| SOS/interaction button | Optional for later interaction; not required by Radar MVP UI | 2 |
| Watch strap | Velcro or equivalent wearable strap | 2 |
| Enclosure | 3D-printed or ABS, with antenna and sensor clearances | 2 |

## Node Assignment

| Hardware | Node A / Anchor | Node B / Tag |
|---|---:|---:|
| ESP32 + BU01 | 1 | 1 |
| ST7789 | 1 | 1 |
| GY-511 | 1 | 1 |
| MAX30102 | 1 | 1 |
| Battery power chain | 1 | 1 |

The former third ESP32/BU01 can remain a spare or diagnostic board. It is not part of the current product architecture.

## Purchase Priorities

1. Complete Node A with the ST7789, GY-511, and MAX30102 already used by Node B.
2. Verify the exact BU01 power pin and antenna orientation on both modules.
3. Verify TPS63802 stability during concurrent UWB and ESP-NOW activity.
4. Confirm the exact display header count and physical dimensions before enclosure or PCB work.
5. Defer vibration and waterproofing until the two-screen Radar MVP passes bench testing.
