#include <unity.h>

#include "EspNowRangeLink.h"

void test_accepts_valid_packet() {
  EspNowRangePacket packet{};
  packet.distanceM = 5.25f;
  packet.headingCdeg = encodeHeadingCdeg(42.5f);
  packet.flags = kEspNowRangeFlagHeadingValid | kEspNowRangeFlagSosActive;

  TEST_ASSERT_TRUE(isValidEspNowRangePacket(&packet, sizeof(packet)));
  TEST_ASSERT_TRUE(packetHasHeading(packet));
  TEST_ASSERT_TRUE(packetHasSos(packet));
  TEST_ASSERT_FLOAT_WITHIN(0.01f, 42.5f, decodeHeadingDeg(packet.headingCdeg));
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

void test_heading_encoding_clamps_to_compact_centidegrees() {
  TEST_ASSERT_EQUAL_UINT16(0, encodeHeadingCdeg(-10.0f));
  TEST_ASSERT_EQUAL_UINT16(0, encodeHeadingCdeg(360.0f));
  TEST_ASSERT_EQUAL_UINT16(35999, encodeHeadingCdeg(359.999f));
}
