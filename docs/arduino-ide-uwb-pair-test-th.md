# คู่มือ Arduino IDE: UWB Pair Test

คู่มือนี้ใช้สำหรับรัน BPMWATCH `uwb_pair_test` ด้วย Arduino IDE แทน PlatformIO

เทสต์นี้เป็น smoke test ขั้นแรกสำหรับโมดูล B&T BU01 DW1000 LDO โดยจะเริ่มต้น DW1000 บน SPI bus หลักของ ESP32 และพิมพ์ค่า mock distance ทุก 1 วินาที เทสต์นี้ยังไม่ใช่ real two-node ranging

## ฮาร์ดแวร์

ใช้ ESP32 1 ตัว และโมดูล B&T BU01 DW1000 LDO 1 ตัว

| สัญญาณ BU01 / DW1000 | ขา ESP32 | หมายเหตุ |
|---|---:|---|
| SCK | GPIO18 | SPI clock หลัก |
| MISO | GPIO19 | SPI MISO หลัก |
| MOSI | GPIO23 | SPI MOSI หลัก |
| CS / SS | GPIO5 | chip select ของ DW1000 |
| IRQ | GPIO34 | ขา interrupt แบบ input-only |
| RST | GPIO4 | reset ของ DW1000 |
| VCC | 3.3V หรือ VIN ที่ยืนยันแล้ว | ดู label บน breakout จริงก่อนใช้ VIN |
| GND | GND | ต้องต่อกราวด์ร่วมกัน |

อย่าต่อขาที่ไม่แน่ใจของ BU01 เข้ากับ 5V ขาสัญญาณของ DW1000 ต้องปลอดภัยกับระดับ 3.3V

## ติดตั้ง Arduino IDE และ ESP32

1. ติดตั้ง Arduino IDE 2.x
2. เพิ่ม ESP32 board support ใน Boards Manager
3. เลือกบอร์ด `ESP32 Dev Module` หรือบอร์ด ESP32 ที่ตรงกับของคุณ
4. ตั้ง Serial Monitor เป็น baud rate `115200`

## ติดตั้ง DW1000 Library ที่ patch แล้ว

อย่าติดตั้ง DW1000 library ตัว upstream ที่ยังไม่ patch จาก Library Manager สำหรับเทสต์นี้ เพราะ library เก่ามีปัญหา build กับ ESP32

ให้ใช้ library ที่ patch แล้วใน repo นี้:

```text
C:\Work\Fastwork\BPMWATCH\lib\DW1000
```

copy folder นี้ไปไว้ใน Arduino libraries folder:

```text
C:\Users\<your-user>\Documents\Arduino\libraries\DW1000
```

หลัง copy แล้ว ให้ตรวจว่ามีไฟล์เหล่านี้:

```text
Documents\Arduino\libraries\DW1000\src\DW1000.cpp
Documents\Arduino\libraries\DW1000\src\DW1000Ranging.cpp
```

ไฟล์ `DW1000.cpp` ที่ patch แล้วต้อง guard `SPI.usingInterrupt(...)` แบบนี้:

```cpp
#if !defined(ESP8266) && !defined(ARDUINO_ARCH_ESP32)
  SPI.usingInterrupt(digitalPinToInterrupt(irq));
#endif
```

ไฟล์ `DW1000Ranging.cpp` ที่ patch แล้วต้องมี fallback return ท้าย `detectMessageType(...)`:

```cpp
return -1;
```

หลัง copy library แล้ว ให้ restart Arduino IDE

## สร้าง Arduino Sketch

Arduino IDE ต้องการไฟล์ `.ino` อยู่ใน folder ที่มีชื่อเดียวกัน

สร้าง folder นี้:

```text
C:\Work\Fastwork\BPMWATCH\arduino\uwb_pair_test_arduino
```

สร้างไฟล์นี้:

```text
C:\Work\Fastwork\BPMWATCH\arduino\uwb_pair_test_arduino\uwb_pair_test_arduino.ino
```

ใส่ code นี้:

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

## Upload และตรวจ output

1. ต่อ ESP32 ด้วย USB
2. เลือก COM port ให้ถูกต้อง
3. กด Upload
4. เปิด Serial Monitor ที่ `115200`

output ที่ควรเห็น:

```text
B&T BU01 DW1000 SPI pair ranging test
DW1000 uses default global SPI.
Keep no-CS ST7789 on a separate SPIClass bus.
DW1000 default-SPI init path complete; ranging protocol still needs calibration.
Mock pair distance: 2.00 m quality=2
```

## Troubleshooting

ถ้าเจอ error นี้:

```text
'class SPIClass' has no member named 'usingInterrupt'
```

แปลว่า Arduino IDE กำลังใช้ DW1000 library ที่ยังไม่ patch ให้ลบ DW1000 library ตัวอื่นออกจาก `Documents\Arduino\libraries` แล้ว copy `lib\DW1000` จาก repo นี้เข้าไปใหม่

ถ้า upload ไม่ผ่าน ให้กดปุ่ม `BOOT` บน ESP32 ค้างไว้ตอนเริ่ม upload แล้วปล่อยหลัง Arduino IDE แสดง `Connecting...`

ถ้า Serial Monitor ไม่มีข้อความ ให้ตรวจ:

- baud rate เป็น `115200`
- เลือก COM port ถูกต้อง
- ลองกด reset บน ESP32 หลังเปิด Serial Monitor

ถ้า DW1000 init ได้ แต่ตอนทำ real ranging ในอนาคตไม่ผ่าน ให้ตรวจ:

- BU01 มีไฟเลี้ยงนิ่งพอ
- สาย SPI สั้นและต่อถูกต้อง
- CS ต่อ GPIO5
- IRQ ต่อ GPIO34
- ทั้งสอง node ใช้ firmware และ timing schedule แบบเดียวกัน

## กฎสำคัญเรื่อง SPI

สำหรับ BPMWATCH ให้ DW1000 ใช้ global `SPI` bus หลัก เพราะ library นี้เรียกใช้ `SPI` โดยตรง

จอ ST7789 แบบไม่มี CS ต้องอยู่บน `SPIClass` อีก bus หนึ่ง อย่าเอา ST7789 no-CS ไปอยู่ SPI bus เดียวกับ DW1000
