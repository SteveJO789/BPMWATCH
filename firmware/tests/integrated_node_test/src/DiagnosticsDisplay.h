#pragma once

#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

#include "DiagnosticsState.h"

class DiagnosticsDisplay {
 public:
  DiagnosticsDisplay();
  void begin(const char* nodeLabel);
  void render(const DiagnosticsState& state, uint32_t nowMs,
              bool forceFullRefresh = false);

 private:
  static constexpr int kDisplaySck = 14;
  static constexpr int kDisplayMosi = 13;
  static constexpr int kDisplayBacklight = 25;
  static constexpr int kDisplayDc = 26;
  static constexpr int kDisplayReset = 27;
  static constexpr int kDisplayCs = -1;

  SPIClass displaySpi_;
  Adafruit_ST7789 tft_;
  bool previousPeerVisible_ = false;
  int previousPeerX_ = 0;
  int previousPeerY_ = 0;
  char nodeLabel_[32] = "";

  void drawRadarStatic(const char* nodeLabel);
  void drawRadarGrid();
  void drawPanel(int y, int height, const char* title);
  void clearPanelValues(int y, int height);
  void renderUwb(const UwbDiagnosticState& state);
  void renderGy511(const Gy511DiagnosticState& state);
  void renderMax30102(const Max30102DiagnosticState& state);
  void renderRadarMap(const DiagnosticsState& state, uint32_t nowMs);
};
