#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

inline void formatI2cAddressList(const uint8_t* addresses, uint8_t count,
                                 char* buffer, size_t bufferSize) {
  if (bufferSize == 0) {
    return;
  }
  if (count == 0 || addresses == nullptr) {
    snprintf(buffer, bufferSize, "NONE");
    return;
  }

  size_t offset = 0;
  buffer[0] = '\0';
  for (uint8_t i = 0; i < count && offset < bufferSize; ++i) {
    const int written = snprintf(buffer + offset, bufferSize - offset,
                                 i == 0 ? "%02X" : " %02X", addresses[i]);
    if (written < 0) {
      break;
    }
    offset += static_cast<size_t>(written);
  }
}
