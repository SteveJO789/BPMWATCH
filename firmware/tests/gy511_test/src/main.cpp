#include <Arduino.h>
#include <BluetoothSerial.h>
#include <Wire.h>
#include <math.h>

constexpr int I2C_SDA = 21;
constexpr int I2C_SCL = 22;
constexpr uint8_t LSM303_ACCEL_ADDR = 0x19;
constexpr uint8_t LSM303_MAG_ADDR = 0x1E;

BluetoothSerial SerialBT;
bool bluetoothReady = false;

void logLine(const char* message) {
  Serial.println(message);
  if (bluetoothReady) {
    SerialBT.println(message);
  }
}

bool writeRegister(uint8_t address, uint8_t reg, uint8_t value) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}

bool readRegisters(uint8_t address, uint8_t reg, uint8_t* buffer, size_t length) {
  Wire.beginTransmission(address);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }

  const size_t received = Wire.requestFrom(static_cast<int>(address), static_cast<int>(length));
  if (received != length) {
    return false;
  }

  for (size_t i = 0; i < length; i++) {
    buffer[i] = Wire.read();
  }
  return true;
}

int16_t le16(uint8_t low, uint8_t high) {
  return static_cast<int16_t>((high << 8) | low);
}

int16_t be16(uint8_t high, uint8_t low) {
  return static_cast<int16_t>((high << 8) | low);
}

bool initGy511() {
  bool ok = true;
  ok &= writeRegister(LSM303_ACCEL_ADDR, 0x20, 0x57);  // 100 Hz, XYZ enabled.
  ok &= writeRegister(LSM303_ACCEL_ADDR, 0x23, 0x00);  // +/-2g, continuous update.
  ok &= writeRegister(LSM303_MAG_ADDR, 0x00, 0x14);    // 15 Hz magnetometer.
  ok &= writeRegister(LSM303_MAG_ADDR, 0x01, 0x20);    // +/-1.3 gauss.
  ok &= writeRegister(LSM303_MAG_ADDR, 0x02, 0x00);    // Continuous conversion.
  return ok;
}

void setup() {
  Serial.begin(115200);
  delay(500);

  SerialBT.enableSSP(false, false);
  bluetoothReady = SerialBT.begin("BPMWATCH-GY511");
  if (!bluetoothReady) {
    Serial.println("Bluetooth SPP init failed; continuing with USB Serial.");
  }

  Wire.begin(I2C_SDA, I2C_SCL);
  logLine("BPMWATCH GY-511 / LSM303DLHC test");
  logLine("Bluetooth device: BPMWATCH-GY511 SSP (no PIN)");
  logLine("Expected addresses: accel=0x19 mag=0x1E");

  if (!initGy511()) {
    logLine("GY-511 init failed. Run i2c_scanner_test and check wiring.");
  } else {
    logLine("GY-511 init OK");
  }
}

void loop() {
  uint8_t accel[6]{};
  uint8_t mag[6]{};

  const bool accelOk = readRegisters(LSM303_ACCEL_ADDR, 0x28 | 0x80, accel, sizeof(accel));
  const bool magOk = readRegisters(LSM303_MAG_ADDR, 0x03, mag, sizeof(mag));

  if (!accelOk || !magOk) {
    logLine("Read failed. Check SDA/SCL, power, and module addresses.");
    delay(1000);
    return;
  }

  const int16_t ax = le16(accel[0], accel[1]) >> 4;
  const int16_t ay = le16(accel[2], accel[3]) >> 4;
  const int16_t az = le16(accel[4], accel[5]) >> 4;

  const int16_t mx = be16(mag[0], mag[1]);
  const int16_t mz = be16(mag[2], mag[3]);
  const int16_t my = be16(mag[4], mag[5]);

  float heading = atan2f(static_cast<float>(my), static_cast<float>(mx)) * 180.0f / PI;
  if (heading < 0.0f) {
    heading += 360.0f;
  }

  char sample[128]{};
  snprintf(sample, sizeof(sample),
           "accel=%d,%d,%d mag=%d,%d,%d heading=%.1f",
           ax, ay, az, mx, my, mz, heading);
  logLine(sample);

  delay(500);
}
