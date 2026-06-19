# คู่มือพัฒนา UWB Pair Ranging และ Web Monitor ผ่าน Wi-Fi AP

คู่มือนี้อธิบายการนำโค้ด UWB pair test ของ S.T.A.T ไปใช้งานกับ ESP32 จำนวน 2 ตัวและโมดูล B&T BU01 DW1000 LDO จำนวน 2 ตัว โดยใช้ SPI สำหรับ UWB และให้ Master สร้าง Wi-Fi AP สำหรับดูค่าระยะผ่านเว็บเบราว์เซอร์

สถานะปัจจุบันเป็นการทดสอบคู่ `Master <-> Slave 1` เท่านั้น ยังไม่ใช่ระบบสามโหนดและยังไม่ได้ปรับ antenna delay เพื่อสอบเทียบความแม่นยำ

## ไฟล์ที่เกี่ยวข้อง

- PlatformIO source: `firmware/tests/uwb_pair_test/src/main.cpp`
- PlatformIO config: `firmware/tests/uwb_pair_test/platformio.ini`
- Arduino IDE sketch: `arduino/uwb_pair_test_arduino/uwb_pair_test_arduino.ino`
- DW1000 library ที่ patch แล้ว: `lib/DW1000`

## หลักการทำงาน

- Master ทำงานเป็น UWB Anchor
- Slave 1 ทำงานเป็น UWB Tag
- ทั้งสอง node ใช้ `DW1000.MODE_LONGDATA_RANGE_ACCURACY`
- เปิด range filter และใช้ค่า filter เท่ากับ `5`
- Master สร้าง Wi-Fi AP ชื่อ `S.T.A.T-UWB`
- เว็บมอนิเตอร์อ่านข้อมูลจาก `/api/status` ทุก 500 ms
- Serial Monitor ของทั้งสอง node แสดง distance, RX power และ quality

ค่าที่กำหนดไว้ในโค้ด:

| รายการ | ค่า |
|---|---|
| Master EUI | `0C:8A:D3:7C:E5:A4:00:01` |
| Slave 1 EUI | `1C:75:C4:F4:E9:D4:00:01` |
| Wi-Fi AP | `S.T.A.T-UWB` |
| Password | `statuwb123` |
| Web monitor | `http://192.168.4.1` |
| Serial baud rate | `115200` |

## การต่อสาย

ต่อ ESP32 กับ BU01 เหมือนกันทั้ง Master และ Slave 1:

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

Build ทั้งสอง role:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e master -e tag
```

Upload Master โดยเปลี่ยน `COM_MASTER` เป็น port จริง:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e master -t upload --upload-port COM_MASTER
```

Upload Slave 1 โดยเปลี่ยน `COM_SLAVE1` เป็น port จริง:

```powershell
& "$env:USERPROFILE\.platformio\penv\Scripts\platformio.exe" run -d firmware\tests\uwb_pair_test -e tag -t upload --upload-port COM_SLAVE1
```

## Upload ด้วย Arduino IDE

1. Copy folder `lib/DW1000` ไปที่ `Documents\Arduino\libraries\DW1000`
2. Restart Arduino IDE
3. เปิด `arduino/uwb_pair_test_arduino/uwb_pair_test_arduino.ino`
4. เลือกบอร์ด `ESP32 Dev Module` และ COM port ของ Master
5. ตั้ง `#define UWB_IS_MASTER 1` แล้ว Upload ลง Master
6. เปลี่ยนเป็น `#define UWB_IS_MASTER 0`
7. เลือก COM port ของ Slave 1 แล้ว Upload
8. เปิด Serial Monitor ที่ `115200`

## วิธีทดสอบ

1. เปิด Master และ Slave 1 โดยวางห่างกันประมาณ 1 เมตรและหันเสาอากาศในแนวเดียวกัน
2. ตรวจ Serial Monitor ของ Master ว่ามี `Role: MASTER / ANCHOR`
3. ตรวจ Serial Monitor ของ Slave 1 ว่ามี `Role: SLAVE 1 / TAG`
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
| M-S1 | 1 | | | | | |
| M-S1 | 2 | | | | | |
| M-S1 | 5 | | | | | |
| M-S1 | 10 | | | | | |

คำนวณ error ด้วยสูตร:

```text
Error = ระยะเฉลี่ยที่วัดได้ - ระยะจริง
```

## Troubleshooting

### ไม่พบ UWB peer

- ตรวจ VCC และ GND ของทั้งสอง node
- ตรวจ SCK, MISO, MOSI, CS, IRQ และ RST
- ตรวจว่าตัวหนึ่งเป็น Master และอีกตัวเป็น Tag
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
- ตรวจ Serial Monitor ของ Master ว่าแสดง IP address
- ปิด mobile data ชั่วคราวถ้าโทรศัพท์พยายามสลับออกจาก Wi-Fi ที่ไม่มีอินเทอร์เน็ต

### ระยะคลาดเคลื่อนมาก

- วางโมดูลให้อยู่นิ่งและเสาอากาศอยู่ในแนวเดียวกัน
- หลีกเลี่ยงโลหะ กำแพง และร่างกายที่บังเส้นทางสัญญาณ
- เก็บค่าหลาย sample แล้วใช้ค่าเฉลี่ย
- ทำ antenna-delay calibration เป็นขั้นตอนถัดไปหลัง real ranging ทำงานต่อเนื่องแล้ว

## เกณฑ์ผ่านขั้นแรก

- Master และ Slave 1 พบกันต่อเนื่อง
- Serial Monitor แสดงค่าระยะจริงโดยไม่ใช่ mock data
- เว็บมอนิเตอร์แสดงข้อมูลเดียวกับ Serial Monitor
- ทดสอบ 1 m, 2 m, 5 m และ 10 m พร้อมบันทึก error, RX power และ quality

เมื่อผ่านเกณฑ์นี้แล้วจึงเริ่ม antenna-delay calibration และทดสอบคู่ `Master <-> Slave 2` กับ `Slave 1 <-> Slave 2`
