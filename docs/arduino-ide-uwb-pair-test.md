# Arduino IDE Guide: UWB Pair Test

Thai version: `docs/arduino-ide-uwb-pair-test-th.md`

This guide is for running the S.T.A.T `uwb_pair_test` with Arduino IDE instead of PlatformIO.

This test performs real two-node ranging between one Master/Anchor and one Slave 1/Tag. The Master also creates the `S.T.A.T-UWB` Wi-Fi access point and serves a live monitor at `http://192.168.4.1`.

## Hardware

Use two ESP32 boards and two B&T BU01 DW1000 LDO modules with the same wiring on both nodes.

| BU01 / DW1000 Signal | ESP32 Pin | Notes |
|---|---:|---|
| SCK | GPIO18 | Default SPI clock |
| MISO | GPIO19 | Default SPI MISO |
| MOSI | GPIO23 | Default SPI MOSI |
| CS / SS | GPIO5 | DW1000 chip select |
| IRQ | GPIO34 | Input-only interrupt pin |
| RST | GPIO4 | DW1000 reset |
| VCC | 3.3V or confirmed module VIN | Check your exact breakout label before using VIN |
| GND | GND | Common ground required |

Do not connect unknown BU01 pins to 5V. The DW1000 signal pins must be 3.3V-safe.

## Install Arduino IDE Support

1. Install Arduino IDE 2.x.
2. Add ESP32 board support in Boards Manager.
3. Select `ESP32 Dev Module` or the matching ESP32 board.
4. Set Serial Monitor baud rate to `115200`.

## Install The Patched DW1000 Library

Do not install the unpatched upstream DW1000 library from Library Manager for this test. The old library has ESP32 build issues.

Use the patched copy already stored in this repo:

```text
C:\Work\Fastwork\BPMWATCH\lib\DW1000
```

Copy that folder into your Arduino libraries folder:

```text
C:\Users\<your-user>\Documents\Arduino\libraries\DW1000
```

After copying, confirm these patched files exist:

```text
Documents\Arduino\libraries\DW1000\src\DW1000.cpp
Documents\Arduino\libraries\DW1000\src\DW1000Ranging.cpp
```

The patched `DW1000.cpp` must guard `SPI.usingInterrupt(...)` like this:

```cpp
#if !defined(ESP8266) && !defined(ARDUINO_ARCH_ESP32)
  SPI.usingInterrupt(digitalPinToInterrupt(irq));
#endif
```

The patched `DW1000Ranging.cpp` must return a fallback value at the end of `detectMessageType(...)`:

```cpp
return -1;
```

Restart Arduino IDE after copying the library.

## Create The Arduino Sketch

Arduino IDE expects an `.ino` file inside a folder with the same name.

The sketch is already available here:

```text
C:\Work\Fastwork\BPMWATCH\arduino\uwb_pair_test_arduino
```

Open `uwb_pair_test_arduino.ino` in Arduino IDE. The role switch is at the top:

```cpp
#define UWB_IS_MASTER 1
```

Use `1` for the Master/Anchor. Use `0` for the Slave 1/Tag.

## Upload And Check Output

1. Set `UWB_IS_MASTER` to `1`, select the Master's COM port, and upload.
2. Set `UWB_IS_MASTER` to `0`, select the Slave 1 COM port, and upload.
3. Open Serial Monitor at `115200` to see connection and range messages.
4. Connect a phone or computer to Wi-Fi `S.T.A.T-UWB` using password `statuwb123`.
5. Open `http://192.168.4.1` to monitor distance, RX power, quality, and update count.

Expected output:

```text
S.T.A.T real UWB pair ranging test
Role: MASTER / ANCHOR
Wi-Fi AP: S.T.A.T-UWB
Password: statuwb123
Monitor: http://192.168.4.1
UWB peer connected: short address 0x...
Range #1: 1.02 m, RX -76.4 dBm, quality 8.0
```

## Troubleshooting

If you see this error:

```text
'class SPIClass' has no member named 'usingInterrupt'
```

Arduino IDE is using an unpatched DW1000 library. Remove other `DW1000` libraries from `Documents\Arduino\libraries`, then copy this repo's `lib\DW1000` folder again.

If upload fails, hold `BOOT` on the ESP32 while upload starts, then release it after Arduino IDE shows `Connecting...`.

If Serial Monitor prints nothing, check:

- Baud rate is `115200`.
- The selected COM port is correct.
- ESP32 reset button was pressed after opening Serial Monitor.

If DW1000 initializes but real ranging fails, check:

- BU01 has stable power.
- All SPI wires are short and correct.
- CS is connected to GPIO5.
- IRQ is connected to GPIO34.
- Both nodes share the same future ranging firmware and timing schedule.

## Important SPI Rule

For BPMWATCH, DW1000 uses the default global `SPI` bus because the library directly owns `SPI`.

The no-CS ST7789 display must stay on a separate `SPIClass` bus. Do not place a no-CS ST7789 display on the same SPI bus as DW1000.
