# คู่มือ Arduino IDE: UWB Pair Test

คู่มือนี้ใช้สำหรับรัน S.T.A.T `uwb_pair_test` ด้วย Arduino IDE แทน PlatformIO

> ขอบเขต diagnostic: เทสต์นี้ยังเก็บ AP monitor และชื่อ role แบบเดิมไว้ แต่ production Radar firmware จะไม่มี AP, webserver หรือ application-level Master/Slave

เทสต์นี้ใช้วัดระยะจริงระหว่าง Node A/Anchor และ Node B/Tag โดย Anchor ฝั่ง diagnostic จะสร้าง Wi-Fi AP ชื่อ `S.T.A.T-UWB` และมีหน้ามอนิเตอร์ที่ `http://192.168.4.1`

## ฮาร์ดแวร์

ใช้ ESP32 2 ตัว และโมดูล B&T BU01 DW1000 LDO 2 ตัว โดยต่อสายเหมือนกันทั้งสอง node

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

ไฟล์ sketch พร้อมใช้งานอยู่ที่:

```text
C:\Work\Fastwork\BPMWATCH\arduino\uwb_pair_test_arduino
```

เปิดไฟล์ `uwb_pair_test_arduino.ino` ด้วย Arduino IDE แล้วเลือก role ที่บรรทัดบนสุด:

```cpp
#define UWB_IS_MASTER 1
```

ใช้ค่า `1` สำหรับ Node A/Anchor และใช้ค่า `0` สำหรับ Node B/Tag ชื่อ macro เดิมยังคงอยู่เพื่อให้ตรงกับ sketch ที่ผ่านการทดสอบแล้ว

## Upload และตรวจ output

1. ตั้ง `UWB_IS_MASTER` เป็น `1` เลือก COM port ของ Node A แล้วกด Upload
2. ตั้ง `UWB_IS_MASTER` เป็น `0` เลือก COM port ของ Node B แล้วกด Upload
3. เปิด Serial Monitor ที่ `115200` เพื่อดูสถานะการเชื่อมต่อและค่าระยะ
4. เชื่อมต่อโทรศัพท์หรือคอมพิวเตอร์กับ Wi-Fi `S.T.A.T-UWB` รหัสผ่าน `statuwb123`
5. เปิด `http://192.168.4.1` เพื่อดู distance, RX power, quality และจำนวนครั้งที่อัปเดต

output ที่ควรเห็น:

ข้อความ `MASTER / ANCHOR` ด้านล่างเป็น string เดิมของ diagnostic firmware และหมายถึง Node A/Anchor ในสถาปัตยกรรมปัจจุบัน

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

ถ้า DW1000 init ได้ แต่ real ranging ไม่ผ่าน ให้ตรวจ:

- BU01 มีไฟเลี้ยงนิ่งพอ
- สาย SPI สั้นและต่อถูกต้อง
- CS ต่อ GPIO5
- IRQ ต่อ GPIO34
- ทั้งสอง node ใช้ firmware และ timing schedule แบบเดียวกัน

## กฎสำคัญเรื่อง SPI

สำหรับ BPMWATCH ให้ DW1000 ใช้ global `SPI` bus หลัก เพราะ library นี้เรียกใช้ `SPI` โดยตรง

จอ ST7789 แบบไม่มี CS ต้องอยู่บน `SPIClass` อีก bus หนึ่ง อย่าเอา ST7789 no-CS ไปอยู่ SPI bus เดียวกับ DW1000

เอกสาร production ปัจจุบันอยู่ที่ `docs/architecture.md`, `docs/pin-map.md`, `docs/wiring-node-a.md` และ `docs/wiring-node-b.md`
