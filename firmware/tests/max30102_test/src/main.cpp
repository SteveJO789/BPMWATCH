#include <Arduino.h>
#include <Wire.h>
#include "MAX30105.h"
#include "heartRate.h"

MAX30105 sensor;

const byte RATE_SIZE = 4;
byte rates[RATE_SIZE];
byte rateSpot = 0;
byte validRateCount = 0;
long lastBeat = 0;
float bpm = 0;
int averageBpm = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!sensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
    return;
  }

  sensor.setup();
  sensor.setPulseAmplitudeRed(0x0A);
  sensor.setPulseAmplitudeGreen(0);
  Serial.println("MAX30102 BPM test: place your finger steadily on the sensor.");
}

void loop() {
  const long irValue = sensor.getIR();

  if (checkForBeat(irValue)) {
    const long now = millis();

    if (lastBeat > 0) {
      const long beatInterval = now - lastBeat;
      const float measuredBpm = 60.0 / (beatInterval / 1000.0);

      if (measuredBpm >= 40 && measuredBpm <= 220) {
        bpm = measuredBpm;
        rates[rateSpot++] = static_cast<byte>(bpm);
        rateSpot %= RATE_SIZE;
        if (validRateCount < RATE_SIZE) {
          validRateCount++;
        }

        averageBpm = 0;
        for (byte i = 0; i < validRateCount; i++) {
          averageBpm += rates[i];
        }
        averageBpm /= validRateCount;
      }
    }

    lastBeat = now;
  }

  Serial.print("IR=");
  Serial.print(irValue);
  Serial.print(", BPM=");
  Serial.print(bpm, 1);
  Serial.print(", Avg BPM=");
  Serial.print(averageBpm);

  if (irValue < 50000) {
    Serial.print(" No finger?");
  }

  Serial.println();
  delay(20);
}
