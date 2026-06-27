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

uint16_t radarDotColor(bool alert) {
  return alert ? ST77XX_RED : ST77XX_GREEN;
}
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
  (void)nodeLabel;
  tft_.setTextColor(ST77XX_CYAN);
  tft_.setTextSize(2);
  tft_.setCursor(8, 8);
  tft_.print("RADAR");
  drawRadarGrid();
}

void DiagnosticsDisplay::drawRadarGrid() {
  tft_.setTextColor(ST77XX_CYAN);
  tft_.setTextSize(1);
  tft_.setCursor(kRadarCenterX - 3, kRadarCenterY - kRadarRadius - 11);
  tft_.print("N");
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
  renderRadarMap(state, nowMs);
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

void DiagnosticsDisplay::renderRadarMap(const DiagnosticsState& state,
                                        uint32_t nowMs) {
  tft_.fillRect(0, 0, 240, 31, ST77XX_BLACK);
  const bool localSos = state.sos.sosActive;
  const bool remoteSos = remoteSosVisible(state.peer.remoteSos, nowMs);
  if (localSos || remoteSos) {
    tft_.fillRect(0, 0, 240, 31, ST77XX_RED);
    tft_.setTextColor(ST77XX_WHITE);
    tft_.setTextSize(2);
    tft_.setCursor(18, 7);
    tft_.print("!!! SOS !!!");
    tft_.setTextSize(1);
    tft_.setCursor(160, 11);
    tft_.print(remoteSos ? "PEER" : "LOCAL");
  } else {
    tft_.setTextColor(ST77XX_CYAN);
    tft_.setTextSize(2);
    tft_.setCursor(8, 8);
    tft_.print("RADAR");
  }

  tft_.fillCircle(kRadarCenterX, kRadarCenterY, kRadarRadius + 4,
                  ST77XX_BLACK);
  previousPeerVisible_ = false;
  drawRadarGrid();

  const bool localBpmLost =
      radarBpmLost(BPMWATCH_ENABLE_MAX30102, state.max30102.initialized,
                   state.max30102.fingerPresent, state.max30102.averageBpm);
  const bool peerBpmLost =
      remoteBpmLostVisible(state.peer.bpmLost, state.peer.lastBpmRxMs, nowMs);
  const bool localAlert = radarNodeAlert(localSos, localBpmLost);
  const bool peerAlert = radarNodeAlert(remoteSos, peerBpmLost);
  const bool headingValid = compassHeadingValid(state.compass);
  if (headingValid) {
    const float headingRad = state.compass.headingDeg * kPi / 180.0f;
    const int headingX =
        kRadarCenterX + static_cast<int>((kRadarRadius - 12) * sinf(headingRad));
    const int headingY =
        kRadarCenterY - static_cast<int>((kRadarRadius - 12) * cosf(headingRad));
    tft_.drawLine(kRadarCenterX, kRadarCenterY, headingX, headingY,
                  ST77XX_YELLOW);
  }
  tft_.fillCircle(kRadarCenterX, kRadarCenterY, 5,
                  radarDotColor(localAlert));

  if (state.radar.hasAngle && !state.radar.hidePeerDot) {
    const float displayAngleDeg = northOrientedRadarAngleDeg(
        state.radar.demoRadarAngleDeg, state.compass.headingDeg, headingValid);
    const float angleRad = displayAngleDeg * kPi / 180.0f;
    const float radiusPx =
        mapDistanceToRadius(state.radar.smoothedDistanceM, kRadarRadius);
    const int peerX =
        kRadarCenterX + static_cast<int>(radiusPx * sinf(angleRad));
    const int peerY =
        kRadarCenterY - static_cast<int>(radiusPx * cosf(angleRad));
    tft_.fillCircle(peerX, peerY, 7, radarDotColor(peerAlert));
    previousPeerX_ = peerX;
    previousPeerY_ = peerY;
    previousPeerVisible_ = true;
  }

  tft_.fillRect(0, kFooterY - 2, 240, 42, ST77XX_BLACK);
  tft_.setTextSize(1);
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(8, kFooterY);
  tft_.print("LINK:");
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
  tft_.setCursor(100, kFooterY);
  tft_.print("D:");
  if (state.uwb.hasRange) {
    tft_.print(state.uwb.distanceM, 1);
    tft_.print("m");
  } else {
    tft_.print("--");
  }

  tft_.setCursor(8, kFooterY + 18);
  tft_.print("BPM:");
  if (!BPMWATCH_ENABLE_MAX30102) {
    tft_.print("OFF");
  } else if (localBpmLost) {
    tft_.setTextColor(ST77XX_RED);
    tft_.print("LOST");
  } else {
    tft_.setTextColor(ST77XX_GREEN);
    tft_.print(state.max30102.averageBpm);
  }
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(78, kFooterY + 18);
  tft_.print("SOS:");
  if (remoteSos) {
    tft_.setTextColor(ST77XX_RED);
    tft_.print("PEER");
  } else if (localSos) {
    tft_.setTextColor(ST77XX_RED);
    tft_.print("LOCAL");
  } else {
    tft_.print("OFF");
  }
  tft_.setTextColor(ST77XX_WHITE);
  tft_.setCursor(140, kFooterY + 18);
  tft_.print("PEER:");
  if (peerBpmLost) {
    tft_.setTextColor(ST77XX_RED);
    tft_.print("BPM");
  } else if (remoteSos) {
    tft_.setTextColor(ST77XX_RED);
    tft_.print("SOS");
  } else if (state.radar.hasAngle && !state.radar.hidePeerDot) {
    tft_.setTextColor(ST77XX_GREEN);
    tft_.print("OK");
  } else {
    tft_.print("--");
  }
}
#endif
