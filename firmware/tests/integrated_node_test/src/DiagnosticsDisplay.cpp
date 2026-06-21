#include "DiagnosticsDisplay.h"

namespace {
constexpr int kUwbY = 34;
constexpr int kUwbHeight = 54;
constexpr int kGyY = 92;
constexpr int kGyHeight = 76;
constexpr int kMaxY = 172;
constexpr int kMaxHeight = 66;
}  // namespace

DiagnosticsDisplay::DiagnosticsDisplay()
    : displaySpi_(HSPI),
      tft_(&displaySpi_, kDisplayCs, kDisplayDc, kDisplayReset) {}

void DiagnosticsDisplay::begin(const char* nodeLabel) {
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
  tft_.setSPISpeed(8000000);
  tft_.setRotation(0);
  tft_.setTextWrap(false);
  tft_.fillScreen(ST77XX_BLACK);

  tft_.setTextColor(ST77XX_CYAN);
  tft_.setTextSize(2);
  tft_.setCursor(8, 8);
  tft_.print(nodeLabel);

  drawPanel(kUwbY, kUwbHeight, "UWB");
  drawPanel(kGyY, kGyHeight, "GY-511");
  drawPanel(kMaxY, kMaxHeight, "MAX30102");
}

void DiagnosticsDisplay::render(const DiagnosticsState& state,
                                uint32_t nowMs) {
  (void)nowMs;
  renderUwb(state.uwb);
  renderGy511(state.gy511);
  renderMax30102(state.max30102);
}

void DiagnosticsDisplay::drawPanel(int y, int height, const char* title) {
  tft_.drawRect(2, y, 236, height, ST77XX_BLUE);
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setTextSize(1);
  tft_.setCursor(8, y + 5);
  tft_.print(title);
}

void DiagnosticsDisplay::clearPanelValues(int y, int height) {
  tft_.fillRect(5, y + 17, 230, height - 20, ST77XX_BLACK);
}

void DiagnosticsDisplay::renderUwb(const UwbDiagnosticState& state) {
  clearPanelValues(kUwbY, kUwbHeight);

  const char* status = "WAIT";
  uint16_t statusColor = ST77XX_ORANGE;
  if (!state.spiReady) {
    status = "SPI ERR";
    statusColor = ST77XX_RED;
  } else if (state.rangeStale) {
    status = "LOST";
    statusColor = ST77XX_RED;
  } else if (state.hasRange) {
    status = "OK";
    statusColor = ST77XX_GREEN;
  }

  tft_.setTextSize(2);
  tft_.setTextColor(statusColor);
  tft_.setCursor(10, kUwbY + 24);
  tft_.print(status);
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(116, kUwbY + 24);
  tft_.print("D:");
  if (state.hasRange) {
    tft_.print(state.distanceM, 2);
    tft_.print("m");
  } else {
    tft_.print("--");
  }
}

void DiagnosticsDisplay::renderGy511(const Gy511DiagnosticState& state) {
  clearPanelValues(kGyY, kGyHeight);
  const bool ok = state.initialized && state.readOk;

  tft_.setTextSize(1);
  tft_.setTextColor(ok ? ST77XX_GREEN : ST77XX_RED);
  tft_.setCursor(10, kGyY + 22);
  tft_.print(ok ? "OK" : "READ ERR");

  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(92, kGyY + 22);
  tft_.print("HEAD:");
  if (ok) {
    tft_.print(state.headingDeg, 1);
    tft_.print(" deg");
  } else {
    tft_.print("--");
  }

  tft_.setCursor(10, kGyY + 43);
  tft_.print("ACC X:");
  if (ok) {
    tft_.print(state.accelX);
    tft_.print(" Y:");
    tft_.print(state.accelY);
    tft_.print(" Z:");
    tft_.print(state.accelZ);
  } else {
    tft_.print("--  Y:--  Z:--");
  }
}

void DiagnosticsDisplay::renderMax30102(
    const Max30102DiagnosticState& state) {
  clearPanelValues(kMaxY, kMaxHeight);

  const char* status = "ERR";
  uint16_t statusColor = ST77XX_RED;
  if (state.initialized && !state.fingerPresent) {
    status = "NO FINGER";
    statusColor = ST77XX_ORANGE;
  } else if (state.initialized) {
    status = "OK";
    statusColor = ST77XX_GREEN;
  }

  tft_.setTextSize(1);
  tft_.setTextColor(statusColor);
  tft_.setCursor(10, kMaxY + 22);
  tft_.print(status);
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(100, kMaxY + 22);
  tft_.print("IR:");
  if (state.initialized) {
    tft_.print(state.irValue);
  } else {
    tft_.print("--");
  }

  tft_.setTextSize(2);
  tft_.setCursor(10, kMaxY + 40);
  tft_.print("AVG BPM: ");
  if (state.fingerPresent && state.averageBpm > 0) {
    tft_.print(state.averageBpm);
  } else {
    tft_.print("--");
  }
}
