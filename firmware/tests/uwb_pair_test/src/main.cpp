#include <Arduino.h>
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
  Serial.println("DW1000 uses default global SPI because thotro/arduino-dw1000 owns SPI directly.");
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
