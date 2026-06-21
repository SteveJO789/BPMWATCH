#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

constexpr int DISPLAY_SCL = 14;  // Module label SCL, SPI SCK on separate display bus.
constexpr int DISPLAY_SDA = 13;  // Module label SDA, SPI MOSI on separate display bus.
constexpr int DISPLAY_BLC = 25;
constexpr int DISPLAY_DC = 26;
constexpr int DISPLAY_RES = 27;
constexpr uint32_t DISPLAY_SPI_HZ = 8000000;

SPIClass displaySpi(HSPI);
Adafruit_ST7789 tft(&displaySpi, -1, DISPLAY_DC, DISPLAY_RES);

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("ZJY-IPS130-V2.0 ST7789 240x240 display test");
  Serial.println("Module labels: SCL=SPI SCK, SDA=SPI MOSI, BLC=backlight, DC=data/command, RES=reset");
  Serial.println("Display uses HSPI/SPIClass so default SPI remains available for DW1000/BU01.");

  pinMode(DISPLAY_BLC, OUTPUT);
  digitalWrite(DISPLAY_BLC, HIGH);

  pinMode(DISPLAY_RES, OUTPUT);
  digitalWrite(DISPLAY_RES, HIGH);
  delay(50);
  digitalWrite(DISPLAY_RES, LOW);
  delay(50);
  digitalWrite(DISPLAY_RES, HIGH);
  delay(50);

  displaySpi.begin(DISPLAY_SCL, -1, DISPLAY_SDA, -1);
  tft.init(240, 240, SPI_MODE3);
  tft.setSPISpeed(DISPLAY_SPI_HZ);
  tft.setRotation(0);

  tft.fillScreen(ST77XX_RED);
  delay(500);
  tft.fillScreen(ST77XX_GREEN);
  delay(500);
  tft.fillScreen(ST77XX_BLUE);
  delay(500);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(18, 20);
  tft.println("BPMWATCH");
  tft.setTextSize(1);
  tft.setCursor(18, 50);
  tft.println("ST7789 240x240");
  tft.drawCircle(120, 135, 70, ST77XX_BLUE);
  tft.fillCircle(120, 135, 5, ST77XX_GREEN);
  tft.fillCircle(82, 112, 5, ST77XX_ORANGE);
  tft.fillCircle(164, 158, 5, ST77XX_RED);
}

void loop() {
  delay(1000);
}
