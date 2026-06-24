#pragma once

inline const char* uwbRfQualityLabel(float firstPathPowerDbm,
                                     float powerDeltaDb) {
  if (powerDeltaDb > 10.0f) {
    return "NLOS";
  }
  if (firstPathPowerDbm < -110.0f) {
    return "WEAK";
  }
  if (firstPathPowerDbm > -105.0f && powerDeltaDb < 6.0f) {
    return "GOOD";
  }
  return "MAYBE";
}
