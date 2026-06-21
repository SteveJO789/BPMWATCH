#include "DisplayUI.h"

namespace {
constexpr uint32_t kDisplaySpiHz = 8000000;

int mapCoord(float value, float minValue, float maxValue, int minPixel, int maxPixel) {
  if (value < minValue) {
    value = minValue;
  }
  if (value > maxValue) {
    value = maxValue;
  }
  const float span = maxValue - minValue;
  if (span <= 0.0f) {
    return minPixel;
  }
  const float normalized = (value - minValue) / span;
  return minPixel + static_cast<int>(normalized * (maxPixel - minPixel));
}
}

DisplayUI::DisplayUI()
    : displaySpi_(HSPI), tft_(&displaySpi_, kDisplayCs, kDisplayDc, kDisplayReset) {}

void DisplayUI::begin() {
  Serial.println("ZJY-IPS130-V2.0 ST7789 240x240 using HSPI/SPIClass");
  Serial.println("Display init: SPI_MODE3, 8 MHz");
  Serial.println("Default SPI is reserved for DW1000/BU01.");

  pinMode(kDisplayBacklight, OUTPUT);
  digitalWrite(kDisplayBacklight, HIGH);

  pinMode(kDisplayReset, OUTPUT);
  digitalWrite(kDisplayReset, HIGH);
  delay(50);
  digitalWrite(kDisplayReset, LOW);
  delay(50);
  digitalWrite(kDisplayReset, HIGH);
  delay(50);

  displaySpi_.begin(kDisplaySck, -1, kDisplayMosi, kDisplayCs);
  tft_.init(240, 240, SPI_MODE3);
  tft_.setSPISpeed(kDisplaySpiHz);
  tft_.setRotation(0);
  tft_.fillScreen(ST77XX_BLACK);
  tft_.setTextWrap(false);
}

void DisplayUI::draw(const TeamMapPacket& packet) {
  Serial.print("Screen map valid: ");
  Serial.print(packet.mapValid);
  Serial.print(" | S1 BPM ");
  Serial.print(packet.slave1Bpm);
  Serial.print(" | S2 BPM ");
  Serial.println(packet.slave2Bpm);

  tft_.fillScreen(ST77XX_BLACK);
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setTextSize(2);
  tft_.setCursor(10, 8);
  tft_.print("BPMWATCH");

  tft_.setTextSize(1);
  tft_.setCursor(10, 34);
  tft_.print(packet.mapValid ? "MAP OK" : "MAP LOST");

  tft_.drawRect(16, 56, 208, 136, ST77XX_BLUE);

  if (packet.mapValid) {
    const uint16_t linkColor = tft_.color565(64, 64, 64);
    const int masterX = mapCoord(packet.masterX, -1.0f, 6.0f, 28, 212);
    const int masterY = mapCoord(packet.masterY, -1.0f, 6.0f, 180, 68);
    const int slave1X = mapCoord(packet.slave1X, -1.0f, 6.0f, 28, 212);
    const int slave1Y = mapCoord(packet.slave1Y, -1.0f, 6.0f, 180, 68);
    const int slave2X = mapCoord(packet.slave2X, -1.0f, 6.0f, 28, 212);
    const int slave2Y = mapCoord(packet.slave2Y, -1.0f, 6.0f, 180, 68);

    tft_.drawLine(masterX, masterY, slave1X, slave1Y, linkColor);
    tft_.drawLine(masterX, masterY, slave2X, slave2Y, linkColor);
    tft_.drawLine(slave1X, slave1Y, slave2X, slave2Y, linkColor);

    drawMapPoint(masterX, masterY, ST77XX_GREEN, "M");
    drawMapPoint(slave1X, slave1Y, ST77XX_ORANGE, "S1");
    drawMapPoint(slave2X, slave2Y, ST77XX_RED, "S2");
  }

  tft_.setTextColor(ST77XX_WHITE);
  tft_.setTextSize(1);
  tft_.setCursor(10, 204);
  tft_.print("S1 ");
  tft_.print(packet.slave1Bpm);
  tft_.print(" BPM ");
  tft_.print(packet.slave1Battery);
  tft_.print("%");

  tft_.setCursor(10, 220);
  tft_.print("S2 ");
  tft_.print(packet.slave2Bpm);
  tft_.print(" BPM ");
  tft_.print(packet.slave2Battery);
  tft_.print("%");
}

void DisplayUI::drawMapPoint(int x, int y, uint16_t color, const char* label) {
  tft_.fillCircle(x, y, 5, color);
  tft_.setTextColor(color);
  tft_.setTextSize(1);
  tft_.setCursor(x + 7, y - 4);
  tft_.print(label);
}
