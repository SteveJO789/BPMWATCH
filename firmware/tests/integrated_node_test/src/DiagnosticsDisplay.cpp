#include "FeatureFlags.h"

#if BPMWATCH_ENABLE_DISPLAY
#include "DiagnosticsDisplay.h"

#include <math.h>
#include <string.h>

#include "NodeConfig.h"

namespace {
constexpr int kUwbY = 34;
constexpr int kUwbHeight = 54;
constexpr int kGyY = 92;
constexpr int kGyHeight = 76;
constexpr int kMaxY = 172;
constexpr int kMaxHeight = 66;
constexpr int kRadarCenterX = 120;
constexpr int kRadarCenterY = 108;
constexpr int kRadarRadius = 82;
constexpr int kFooterY = 198;
constexpr float kPi = 3.14159265358979323846f;
}  // namespace

DiagnosticsDisplay::DiagnosticsDisplay()
    : displaySpi_(HSPI),
      tft_(&displaySpi_, kDisplayCs, kDisplayDc, kDisplayReset) {}

void DiagnosticsDisplay::begin(const char* nodeLabel) {
  strncpy(nodeLabel_, nodeLabel, sizeof(nodeLabel_) - 1);
  nodeLabel_[sizeof(nodeLabel_) - 1] = '\0';

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
  Serial.println("DISPLAY boot test: init ST7789");
  tft_.init(240, 240, SPI_MODE3);
  tft_.setSPISpeed(8000000);
  tft_.setRotation(0);
  tft_.setTextWrap(false);
  Serial.println("DISPLAY boot test: RED");
  tft_.fillScreen(ST77XX_RED);
  delay(150);
  Serial.println("DISPLAY boot test: GREEN");
  tft_.fillScreen(ST77XX_GREEN);
  delay(150);
  Serial.println("DISPLAY boot test: BLUE");
  tft_.fillScreen(ST77XX_BLUE);
  delay(150);
  tft_.fillScreen(ST77XX_BLACK);
  drawRadarStatic(nodeLabel);
  Serial.println("DISPLAY boot test: DONE");
}

void DiagnosticsDisplay::drawRadarStatic(const char* nodeLabel) {
  tft_.setTextColor(ST77XX_CYAN);
  tft_.setTextSize(2);
  tft_.setCursor(8, 8);
  tft_.print("RADAR ");
  tft_.print(nodeLabel);
  drawRadarGrid();
}

void DiagnosticsDisplay::drawRadarGrid() {
  tft_.drawCircle(kRadarCenterX, kRadarCenterY, kRadarRadius, ST77XX_BLUE);
  tft_.drawCircle(kRadarCenterX, kRadarCenterY, kRadarRadius / 2,
                  ST77XX_BLUE);
  tft_.drawFastHLine(kRadarCenterX - kRadarRadius, kRadarCenterY,
                     kRadarRadius * 2, ST77XX_BLUE);
  tft_.drawFastVLine(kRadarCenterX, kRadarCenterY - kRadarRadius,
                     kRadarRadius * 2, ST77XX_BLUE);
  tft_.fillCircle(kRadarCenterX, kRadarCenterY, 5, ST77XX_GREEN);
}

void DiagnosticsDisplay::render(const DiagnosticsState& state,
                                uint32_t nowMs,
                                bool forceFullRefresh) {
  (void)nowMs;
  if (forceFullRefresh) {
    previousPeerVisible_ = false;
    tft_.fillScreen(ST77XX_BLACK);
    drawRadarStatic(nodeLabel_);
  }
  renderRadarMap(state);
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
  const bool hasAccel = ok && state.accelAvailable;

  tft_.setTextSize(1);
  tft_.setTextColor(ok ? ST77XX_GREEN : ST77XX_RED);
  tft_.setCursor(10, kGyY + 22);
  tft_.print(gy511StatusLabel(state.status));

  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(92, kGyY + 22);
  tft_.print("HEAD:");
  if (ok) {
    tft_.print(state.headingDeg, 1);
    tft_.print(" deg");
  } else {
    tft_.print("--");
  }

  if (hasAccel) {
    tft_.setCursor(10, kGyY + 43);
    tft_.print("ACC X:");
    tft_.print(state.accelX);
    tft_.print(" Y:");
    tft_.print(state.accelY);
    tft_.print(" Z:");
    tft_.print(state.accelZ);
  } else if (ok) {
    tft_.setCursor(10, kGyY + 43);
    tft_.print("MAG X:");
    tft_.print(state.magX);
    tft_.print(" Y:");
    tft_.print(state.magY);
    tft_.print(" Z:");
    tft_.print(state.magZ);
  } else {
    tft_.setCursor(10, kGyY + 43);
    tft_.print("ACC X:");
    tft_.print("--  Y:--  Z:--");
  }

  tft_.setCursor(10, kGyY + 61);
  tft_.print("I2C:");
  tft_.print(state.i2cAddresses);
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

void DiagnosticsDisplay::renderRadarMap(const DiagnosticsState& state) {
  if (previousPeerVisible_) {
    tft_.fillCircle(previousPeerX_, previousPeerY_, 8, ST77XX_BLACK);
    previousPeerVisible_ = false;
  }
  drawRadarGrid();

  if (state.radar.hasAngle && !state.radar.hidePeerDot) {
    const float angleRad = state.radar.peerAngleDeg * kPi / 180.0f;
    const float radiusPx =
        mapDistanceToRadius(state.radar.smoothedDistanceM, kRadarRadius);
    const int peerX =
        kRadarCenterX + static_cast<int>(radiusPx * sinf(angleRad));
    const int peerY =
        kRadarCenterY - static_cast<int>(radiusPx * cosf(angleRad));
    tft_.fillCircle(peerX, peerY, 7, ST77XX_ORANGE);
    previousPeerX_ = peerX;
    previousPeerY_ = peerY;
    previousPeerVisible_ = true;
  }

  tft_.fillRect(0, kFooterY - 2, 240, 42, ST77XX_BLACK);
  tft_.setTextSize(1);
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(8, kFooterY);
  tft_.print("UWB:");
  const char* uwbStatus = radarLinkStatusLabel(state.uwb.spiReady, state.radar);
  if (strcmp(uwbStatus, "SPI ERR") == 0) {
    tft_.setTextColor(ST77XX_RED);
  } else if (strcmp(uwbStatus, "LOST") == 0) {
    tft_.setTextColor(ST77XX_RED);
  } else if (strcmp(uwbStatus, "OK") == 0) {
    tft_.setTextColor(ST77XX_GREEN);
  } else {
    tft_.setTextColor(ST77XX_ORANGE);
  }
  tft_.print(uwbStatus);

  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(92, kFooterY);
  tft_.print("D:");
  if (state.uwb.hasRange) {
    tft_.print(state.uwb.distanceM, 1);
    tft_.print("m");
  } else {
    tft_.print("--");
  }

  tft_.setCursor(8, kFooterY + 18);
  tft_.print("BPM:");
  if (!state.max30102.initialized) {
    tft_.setTextColor(ST77XX_RED);
    tft_.print("ERR");
  } else if (!state.max30102.fingerPresent) {
    tft_.setTextColor(ST77XX_ORANGE);
    tft_.print("TOUCH");
  } else if (state.max30102.averageBpm > 0) {
    tft_.setTextColor(ST77XX_GREEN);
    tft_.print(state.max30102.averageBpm);
  } else {
    tft_.setTextColor(ST77XX_WHITE);
    tft_.print("--");
  }
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(92, kFooterY + 18);
  tft_.print("Angle:");
  tft_.print(state.radar.peerAngleDeg, 0);
}
#endif
