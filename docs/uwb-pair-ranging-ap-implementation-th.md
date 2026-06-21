# คู่มือ Diagnostic UWB Pair Ranging และ Web Monitor ผ่าน Wi-Fi AP

คู่มือนี้อธิบาย diagnostic `uwb_pair_test` ของ S.T.A.T สำหรับ ESP32 2 ตัวและ B&T BU01 DW1000 LDO 2 ตัว โดยใช้ SPI สำหรับ UWB และให้ฝั่ง Anchor สร้าง Wi-Fi AP สำหรับดูค่าระยะผ่านเว็บเบราว์เซอร์

> AP และ web monitor ในเอกสารนี้เป็นเครื่องมือ diagnostic เท่านั้น Production Radar firmware ใช้ Node A/Node B, ไม่มี application-level Master/Slave และไม่มี AP/webserver

คู่ที่ทดสอบแล้วคือ `Node A/Anchor <-> Node B/Tag` โดยใช้ antenna delay `16555` และ application median filter 5 samples

## ไฟล์ที่เกี่ยวข้อง

- PlatformIO source: `firmware/tests/uwb_pair_test/src/main.cpp`
- PlatformIO config: `firmware/tests/uwb_pair_test/platformio.ini`
- Arduino IDE sketch: `arduino/uwb_pair_test_arduino/uwb_pair_test_arduino.ino`
- DW1000 library ที่ patch แล้ว: `lib/DW1000`

## หลักการทำงาน

- Node A ทำงานเป็น UWB Anchor
- Node B ทำงานเป็น UWB Tag
- ทั้งสอง node ใช้ `DW1000.MODE_LONGDATA_RANGE_ACCURACY`
- calibrated build ใช้ antenna delay `16555` และ application median filter 5 samples
- Anchor recovery ใช้ timeout 5 วินาทีก่อนเคยเชื่อมต่อ และ 15 วินาทีหลังเคยเชื่อมต่อ
- เว็บแสดง `STALE` พร้อมค่าล่าสุดระหว่าง packet gap และนับ sample ที่ถูกปฏิเสธ
- Anchor ฝั่ง diagnostic สร้าง Wi-Fi AP ชื่อ `S.T.A.T-UWB`
- เว็บมอนิเตอร์อ่านข้อมูลจาก `/api/status` ทุก 500 ms
- Serial Monitor ของทั้งสอง node แสดง distance, RX power และ quality

ค่าที่กำหนดไว้ในโค้ด:

| รายการ | ค่า |
|---|---|
| Node A / Anchor EUI | `0C:8A:D3:7C:E5:A4:00:01` |
| Node B / Tag EUI | `1C:75:C4:F4:E9:D4:00:01` |
| Wi-Fi AP | `S.T.A.T-UWB` |
| Password | `statuwb123` |
| Web monitor | `http://192.168.4.1` |
| Serial baud rate | `115200` |
| Calibration antenna delay | `16555` |
| Range filter | Application median 5 samples |
| Anchor recovery timeout | Discovery `5000 ms`, connected `15000 ms` |

## การต่อสาย

ต่อ ESP32 กับ BU01 เหมือนกันทั้ง Node A และ Node B:

| BU01 / DW1000 | ESP32 |
|---|---:|
| SCK | GPIO18 |
| MISO | GPIO19 |
| MOSI | GPIO23 |
| CS / SS | GPIO5 |
| IRQ | GPIO34 |
| RST | GPIO4 |
| GND | GND |
| VCC | 3.3V หรือ VIN ที่ยืนยันจากโมดูลแล้ว |

ขาสัญญาณ DW1000 ต้องใช้ระดับ 3.3V และควรใช้สาย SPI ที่สั้น ห้ามนำจอ ST7789 แบบไม่มี CS มาใช้ SPI bus เดียวกับ DW1000 จอต้องอยู่บน `SPIClass` อีก bus หนึ่ง

## Upload ด้วย PlatformIO

ไฟล์ `platformio.ini` มี environment แยกสองตัว:

- `master`: กำหนด `UWB_IS_MASTER=1`
- `tag`: กำหนด `UWB_IS_MASTER=0`

ชื่อ environment และ macro เป็นชื่อเดิมของ test ที่ผ่านการทดสอบแล้ว โดย `master` หมายถึง Node A/Anchor และ `tag` หมายถึง Node B/Tag

Build ทั้งสอง role:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e master -e tag
```

Upload Node A/Anchor โดยเปลี่ยน `COM_NODE_A` เป็น port จริง:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e master -t upload --upload-port COM_NODE_A
```

Upload Node B/Tag โดยเปลี่ยน `COM_NODE_B` เป็น port จริง:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e tag -t upload --upload-port COM_NODE_B
```

## Upload ด้วย Arduino IDE

1. Copy folder `lib/DW1000` ไปที่ `Documents\Arduino\libraries\DW1000`
2. Restart Arduino IDE
3. เปิด `arduino/uwb_pair_test_arduino/uwb_pair_test_arduino.ino`
4. เลือกบอร์ด `ESP32 Dev Module` และ COM port ของ Node A
5. ตั้ง `#define UWB_IS_MASTER 1` แล้ว Upload ลง Node A
6. เปลี่ยนเป็น `#define UWB_IS_MASTER 0`
7. เลือก COM port ของ Node B แล้ว Upload
8. เปิด Serial Monitor ที่ `115200`

## วิธีทดสอบ

1. เปิด Node A และ Node B โดยวางห่างกันประมาณ 1 เมตรและหันเสาอากาศในแนวเดียวกัน
2. ตรวจ Serial Monitor ของ Node A ว่ามี `Role: MASTER / ANCHOR` ซึ่งเป็นข้อความเดิมของ diagnostic firmware
3. ตรวจ Serial Monitor ของ Node B ว่ามี `Role: SLAVE 1 / TAG` ซึ่งเป็นข้อความเดิมของ diagnostic firmware
4. รอข้อความ `UWB peer connected`
5. ตรวจว่ามีข้อความ `Range #...` ต่อเนื่อง
6. เชื่อมต่อโทรศัพท์หรือคอมพิวเตอร์กับ Wi-Fi `S.T.A.T-UWB`
7. ใส่รหัสผ่าน `statuwb123`
8. เปิด `http://192.168.4.1`
9. ตรวจว่า distance, RX power, quality และ range updates เปลี่ยนตาม Serial Monitor
10. ทดสอบซ้ำที่ระยะจริง 1 m, 2 m, 5 m และ 10 m

ตัวอย่าง Serial output:

```text
S.T.A.T real UWB pair ranging test
Role: MASTER / ANCHOR
Wi-Fi AP: S.T.A.T-UWB
Password: statuwb123
Monitor: http://192.168.4.1
UWB peer connected: short address 0x751C
Range #1: 1.02 m, RX -76.4 dBm, quality 8.0
```

## การบันทึกผล

บันทึกผลแต่ละระยะลง `test-logs/uwb-pair-test.md` โดยใช้ตารางนี้:

| Pair | ระยะจริง (m) | ระยะเฉลี่ย (m) | Error (m) | RX power (dBm) | Quality | หมายเหตุ |
|---|---:|---:|---:|---:|---:|---|
| Node A-Node B | 1 | | | | | |
| Node A-Node B | 2 | | | | | |
| Node A-Node B | 5 | | | | | |
| Node A-Node B | 10 | | | | | |

คำนวณ error ด้วยสูตร:

```text
Error = ระยะเฉลี่ยที่วัดได้ - ระยะจริง
```

## Troubleshooting

### ไม่พบ UWB peer

- ตรวจ VCC และ GND ของทั้งสอง node
- ตรวจ SCK, MISO, MOSI, CS, IRQ และ RST
- ตรวจว่า Node A ใช้ Anchor build และ Node B ใช้ Tag build
- ตรวจว่าทั้งสองตัวใช้ mode และ library รุ่นเดียวกัน
- ลดระยะเหลือประมาณ 0.5-1 เมตรเพื่อเริ่มทดสอบ

### มี peer แต่ไม่มีค่า Range

- ใช้แหล่งจ่ายไฟที่นิ่งและสาย USB ที่จ่ายกระแสได้พอ
- ใช้สาย SPI ให้สั้น
- กด Reset ทั้งสอง node หลัง Upload
- ตรวจ Serial Monitor ที่ `115200`

### เข้าเว็บไม่ได้

- ตรวจว่าเชื่อมต่อ Wi-Fi `S.T.A.T-UWB` ไม่ใช่ Wi-Fi อื่น
- เปิด `http://192.168.4.1` โดยใช้ `http` ไม่ใช่ `https`
- ตรวจ Serial Monitor ของ Node A/Anchor ว่าแสดง IP address
- ปิด mobile data ชั่วคราวถ้าโทรศัพท์พยายามสลับออกจาก Wi-Fi ที่ไม่มีอินเทอร์เน็ต

### ระยะคลาดเคลื่อนมาก

- วางโมดูลให้อยู่นิ่งและเสาอากาศอยู่ในแนวเดียวกัน
- หลีกเลี่ยงโลหะ กำแพง และร่างกายที่บังเส้นทางสัญญาณ
- เก็บ raw range หลาย sample ที่ระยะจริง 1 m, 2 m และ 5 m โดยปิด range filter
- ปรับ `UWB_ANTENNA_DELAY` หลังยืนยันว่า error เป็น systematic bias
- เปิด range filter กลับหลัง calibration ผ่านแล้วเท่านั้น

## เกณฑ์ผ่านขั้นแรก

- Node A และ Node B พบกันต่อเนื่อง
- Serial Monitor แสดงค่าระยะจริงโดยไม่ใช่ mock data
- เว็บมอนิเตอร์แสดงข้อมูลเดียวกับ Serial Monitor
- ทดสอบ 1 m, 2 m, 5 m และ 10 m พร้อมบันทึก error, RX power และ quality

ผล calibration ปัจจุบันถูกบันทึกไว้ใน `test-logs/uwb-pair-test.md` ขั้นต่อไปคือยืนยัน ranging เดิมหลังติดตั้ง ST7789, GY-511 และ MAX30102 ครบทั้งสอง node ก่อน integrate production Radar firmware
