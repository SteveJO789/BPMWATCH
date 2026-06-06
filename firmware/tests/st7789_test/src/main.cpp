#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// TODO: Confirm pins on real hardware.
constexpr int TFT_CS = 5;
constexpr int TFT_DC = 16;
constexpr int TFT_RST = 17;

Adafruit_ST7789 tft(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200);
  Serial.println("ST7789 test");

  tft.init(240, 240);
  tft.setRotation(0);
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(2);
  tft.setCursor(20, 20);
  tft.println("BPMWATCH");
  tft.drawCircle(120, 130, 70, ST77XX_BLUE);
  tft.fillCircle(120, 130, 4, ST77XX_GREEN);
  tft.fillCircle(80, 110, 4, ST77XX_ORANGE);
  tft.fillCircle(165, 155, 4, ST77XX_RED);
}

void loop() {
  delay(1000);
}
