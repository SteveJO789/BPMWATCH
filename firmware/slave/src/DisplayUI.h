#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "Packets.h"

class DisplayUI {
 public:
  DisplayUI();
  void begin();
  void draw(const TeamMapPacket& packet);

 private:
  static constexpr int kDisplaySck = 14;
  static constexpr int kDisplayMosi = 13;
  static constexpr int kDisplayBacklight = 25;
  static constexpr int kDisplayDc = 26;
  static constexpr int kDisplayReset = 27;
  static constexpr int kDisplayCs = -1;

  SPIClass displaySpi_;
  Adafruit_ST7789 tft_;

  void drawMapPoint(int x, int y, uint16_t color, const char* label);
};
