# Arduino IDE Guide: UWB Pair Test

Thai version: `docs/arduino-ide-uwb-pair-test-th.md`

This guide is for running the BPMWATCH `uwb_pair_test` with Arduino IDE instead of PlatformIO.

The test is a first bring-up smoke test for the B&T BU01 DW1000 LDO module. It initializes DW1000 on the ESP32 default SPI bus and prints a mock distance once per second. It does not perform real two-node ranging yet.

## Hardware

Use one ESP32 and one B&T BU01 DW1000 LDO module.

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

Create this folder:

```text
C:\Work\Fastwork\BPMWATCH\arduino\uwb_pair_test_arduino
```

Create this file:

```text
C:\Work\Fastwork\BPMWATCH\arduino\uwb_pair_test_arduino\uwb_pair_test_arduino.ino
```

Paste this code:

```cpp
#include <DW1000.h>
#include <SPI.h>

constexpr int UWB_SCK = 18;
constexpr int UWB_MISO = 19;
constexpr int UWB_MOSI = 23;
constexpr int UWB_CS = 5;
constexpr int UWB_IRQ = 34;
constexpr int UWB_RST = 4;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("B&T BU01 DW1000 SPI pair ranging test");
  Serial.println("DW1000 uses default global SPI.");
  Serial.println("Keep no-CS ST7789 on a separate SPIClass bus.");

  SPI.begin(UWB_SCK, UWB_MISO, UWB_MOSI, UWB_CS);
  DW1000.begin(UWB_IRQ, UWB_RST);
  DW1000.select(UWB_CS);

  Serial.println("DW1000 default-SPI init path complete; ranging protocol still needs calibration.");
}

void loop() {
  static uint32_t lastMs = 0;
  if (millis() - lastMs >= 1000) {
    lastMs = millis();
    Serial.println("Mock pair distance: 2.00 m quality=2");
  }
}
```

## Upload And Check Output

1. Connect the ESP32 by USB.
2. Select the correct COM port.
3. Click Upload.
4. Open Serial Monitor at `115200`.

Expected output:

```text
B&T BU01 DW1000 SPI pair ranging test
DW1000 uses default global SPI.
Keep no-CS ST7789 on a separate SPIClass bus.
DW1000 default-SPI init path complete; ranging protocol still needs calibration.
Mock pair distance: 2.00 m quality=2
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

If DW1000 init runs but later real ranging fails, check:

- BU01 has stable power.
- All SPI wires are short and correct.
- CS is connected to GPIO5.
- IRQ is connected to GPIO34.
- Both nodes share the same future ranging firmware and timing schedule.

## Important SPI Rule

For BPMWATCH, DW1000 uses the default global `SPI` bus because the library directly owns `SPI`.

The no-CS ST7789 display must stay on a separate `SPIClass` bus. Do not place a no-CS ST7789 display on the same SPI bus as DW1000.
