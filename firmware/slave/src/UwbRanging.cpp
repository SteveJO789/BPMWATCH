#include "UwbRanging.h"

#include <DW1000.h>
#include <SPI.h>

namespace {
constexpr int kUwbSck = 18;
constexpr int kUwbMiso = 19;
constexpr int kUwbMosi = 23;
constexpr int kUwbCs = 5;
constexpr int kUwbIrq = 34;
constexpr int kUwbRst = 4;
}

void UwbRanging::begin() {
  Serial.println("B&T BU01 DW1000 using default SPI");
  SPI.begin(kUwbSck, kUwbMiso, kUwbMosi, kUwbCs);
  DW1000.begin(kUwbIrq, kUwbRst);
  DW1000.select(kUwbCs);
  // TODO: Join scheduled DW1000 ranging slots.
}

void UwbRanging::poll() {
  // TODO: Participate in scheduled UWB ranging slots.
}
