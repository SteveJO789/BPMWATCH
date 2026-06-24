#include <unity.h>

#include "EspNowRangeLink.h"

void test_accepts_valid_packet() {
  EspNowRangePacket packet{};
  packet.distanceM = 5.25f;

  TEST_ASSERT_TRUE(isValidEspNowRangePacket(&packet, sizeof(packet)));
}

void test_rejects_wrong_magic() {
  EspNowRangePacket packet{};
  packet.magic = 0;

  TEST_ASSERT_FALSE(isValidEspNowRangePacket(&packet, sizeof(packet)));
}

void test_rejects_wrong_version() {
  EspNowRangePacket packet{};
  packet.version = kEspNowRangeVersion + 1;

  TEST_ASSERT_FALSE(isValidEspNowRangePacket(&packet, sizeof(packet)));
}

void test_rejects_wrong_size() {
  EspNowRangePacket packet{};

  TEST_ASSERT_FALSE(isValidEspNowRangePacket(&packet, sizeof(packet) - 1));
}
