#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <math.h>

#include "RelativeMapSolver.h"

namespace {
constexpr uint32_t kFrameMs = 1000;
constexpr int DISPLAY_SCL = 18;  // Module label SCL, SPI SCK.
constexpr int DISPLAY_SDA = 23;  // Module label SDA, SPI MOSI.
constexpr int DISPLAY_BLC = 25;
constexpr int DISPLAY_DC = 26;
constexpr int DISPLAY_RES = 27;
constexpr int DISPLAY_SIM_CS = 5;  // Wokwi ILI9341 surrogate needs CS.
constexpr int INPUT_D_M1 = 34;
constexpr int INPUT_D_M2 = 35;
constexpr int INPUT_D_12 = 32;
constexpr int INPUT_BPM_1 = 33;
constexpr int INPUT_BPM_2 = 36;
constexpr int INPUT_HEADING = 39;

RelativeMapSolver solver;
Adafruit_ILI9341 tft(DISPLAY_SIM_CS, DISPLAY_DC);

float readScaledAnalog(int pin, float minValue, float maxValue) {
  const int raw = analogRead(pin);
  return minValue + ((maxValue - minValue) * raw / 4095.0f);
}

SlaveStatusPacket makeSlave(uint8_t id, int baseBpm, int baseBattery) {
  SlaveStatusPacket packet{};
  packet.id = id;
  packet.bpm = baseBpm;
  packet.battery = baseBattery - static_cast<int>((millis() / 10000UL) % 8);
  packet.signal = 100;
  return packet;
}

void printDisplayPinPlan() {
  Serial.println("Display hardware target:");
  Serial.println("- IPS TFT LCD 240x240 with ST7789 controller");
  Serial.println("- Module labels: SCL=SPI SCK, SDA=SPI MOSI, BLC=backlight, DC=data/command, RES=reset");
  Serial.println("- Wokwi uses ILI9341 as a visual surrogate because Wokwi has no built-in ST7789 part");
  Serial.println("Adaptive Wokwi inputs:");
  Serial.println("- slide dM1: Master to Slave 1 distance");
  Serial.println("- slide dM2: Master to Slave 2 distance");
  Serial.println("- slide d12: Slave 1 to Slave 2 distance");
  Serial.println("- slide BPM1/BPM2: simulated MAX30102 readings");
  Serial.println("- slide HDG: simulated GY-511 heading");
}

void drawSerialMap(const TeamMapPacket& map, float headingDegrees, float dM1, float dM2,
                   float d12) {
  Serial.print("mapValid=");
  Serial.print(map.mapValid);
  Serial.print(" d=");
  Serial.print(dM1, 2);
  Serial.print("/");
  Serial.print(dM2, 2);
  Serial.print("/");
  Serial.print(d12, 2);
  Serial.print(" M=(");
  Serial.print(map.masterX, 2);
  Serial.print(",");
  Serial.print(map.masterY, 2);
  Serial.print(") S1=(");
  Serial.print(map.slave1X, 2);
  Serial.print(",");
  Serial.print(map.slave1Y, 2);
  Serial.print(") S2=(");
  Serial.print(map.slave2X, 2);
  Serial.print(",");
  Serial.print(map.slave2Y, 2);
  Serial.print(") BPM=");
  Serial.print(map.slave1Bpm);
  Serial.print("/");
  Serial.print(map.slave2Bpm);
  Serial.print(" BAT=");
  Serial.print(map.masterBattery);
  Serial.print("/");
  Serial.print(map.slave1Battery);
  Serial.print("/");
  Serial.print(map.slave2Battery);
  Serial.print(" heading=");
  Serial.println(headingDegrees, 1);
}

int mapToScreen(float valueMeters, float minMeters, float maxMeters, int minPixel, int maxPixel) {
  const float clamped = constrain(valueMeters, minMeters, maxMeters);
  return minPixel + static_cast<int>(((clamped - minMeters) * (maxPixel - minPixel)) /
                                     (maxMeters - minMeters));
}

void drawDisplayMap(const TeamMapPacket& map, float headingDegrees) {
  tft.fillScreen(ILI9341_BLACK);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(8, 8);
  tft.print("BPMWATCH");
  tft.setCursor(8, 32);
  tft.print("BPM:");
  tft.print(map.slave1Bpm);
  tft.print("/");
  tft.print(map.slave2Bpm);
  tft.setCursor(8, 56);
  tft.print("HDG:");
  tft.print(headingDegrees, 0);

  if (!map.mapValid) {
    tft.setTextColor(ILI9341_RED);
    tft.setTextSize(2);
    tft.setCursor(46, 132);
    tft.print("MAP LOST");
    return;
  }

  constexpr int minXY = 54;
  constexpr int maxXY = 218;
  const int mx = mapToScreen(map.masterX, -0.5f, 4.0f, minXY, maxXY);
  const int my = maxXY - mapToScreen(map.masterY, -0.5f, 4.0f, minXY, maxXY) + minXY;
  const int s1x = mapToScreen(map.slave1X, -0.5f, 4.0f, minXY, maxXY);
  const int s1y = maxXY - mapToScreen(map.slave1Y, -0.5f, 4.0f, minXY, maxXY) + minXY;
  const int s2x = mapToScreen(map.slave2X, -0.5f, 4.0f, minXY, maxXY);
  const int s2y = maxXY - mapToScreen(map.slave2Y, -0.5f, 4.0f, minXY, maxXY) + minXY;

  tft.drawLine(mx, my, s1x, s1y, ILI9341_BLUE);
  tft.drawLine(mx, my, s2x, s2y, ILI9341_BLUE);
  tft.drawLine(s1x, s1y, s2x, s2y, ILI9341_BLUE);
  tft.fillCircle(mx, my, 7, ILI9341_GREEN);
  tft.fillCircle(s1x, s1y, 7, ILI9341_ORANGE);
  tft.fillCircle(s2x, s2y, 7, ILI9341_RED);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(2);
  tft.setCursor(mx + 10, my - 8);
  tft.print("M");
  tft.setCursor(s1x + 10, s1y - 8);
  tft.print("S1");
  tft.setCursor(s2x + 10, s2y - 8);
  tft.print("S2");
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(200);
  pinMode(DISPLAY_BLC, OUTPUT);
  pinMode(DISPLAY_RES, OUTPUT);
  digitalWrite(DISPLAY_BLC, HIGH);
  digitalWrite(DISPLAY_RES, LOW);
  delay(50);
  digitalWrite(DISPLAY_RES, HIGH);
  SPI.begin(DISPLAY_SCL, -1, DISPLAY_SDA, DISPLAY_SIM_CS);
  tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);

  Serial.println("BPMWATCH Wokwi simulation");
  Serial.println("Mocks BU01 UWB distances, MAX30102 BPM, GY-511 heading, and battery data.");
  printDisplayPinPlan();
}

void loop() {
  static uint32_t lastFrame = 0;
  if (millis() - lastFrame < kFrameMs) {
    return;
  }
  lastFrame = millis();

  const int bpm1 = static_cast<int>(readScaledAnalog(INPUT_BPM_1, 45.0f, 160.0f));
  const int bpm2 = static_cast<int>(readScaledAnalog(INPUT_BPM_2, 45.0f, 160.0f));
  const SlaveStatusPacket slave1 = makeSlave(NODE_SLAVE_1, bpm1, 92);
  const SlaveStatusPacket slave2 = makeSlave(NODE_SLAVE_2, bpm2, 88);

  const float masterToSlave1 = readScaledAnalog(INPUT_D_M1, 0.5f, 6.0f);
  const float masterToSlave2 = readScaledAnalog(INPUT_D_M2, 0.5f, 6.0f);
  const float slave1ToSlave2 = readScaledAnalog(INPUT_D_12, 0.5f, 6.0f);

  TeamMapPacket map = solver.solve(masterToSlave1, masterToSlave2, slave1ToSlave2, slave1,
                                   slave2, 95);

  const float headingDegrees = readScaledAnalog(INPUT_HEADING, 0.0f, 359.0f);
  drawDisplayMap(map, headingDegrees);
  drawSerialMap(map, headingDegrees, masterToSlave1, masterToSlave2, slave1ToSlave2);
}
