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

void test_bpm_encoding_clamps_to_packet_range() {
  TEST_ASSERT_EQUAL_UINT16(0, encodeBpm(0));
  TEST_ASSERT_EQUAL_UINT16(1, encodeBpm(1));
  TEST_ASSERT_EQUAL_UINT16(255, encodeBpm(300));
}

void test_packet_reports_bpm_valid_and_lost_flags() {
  EspNowRangePacket packet{};
  packet.bpm = encodeBpm(78);
  packet.flags = kEspNowRangeFlagBpmValid;

  TEST_ASSERT_TRUE(packetHasBpm(packet));
  TEST_ASSERT_FALSE(packetHasBpmLost(packet));
  TEST_ASSERT_EQUAL_UINT16(78, packet.bpm);

  packet.flags = kEspNowRangeFlagBpmLost;

  TEST_ASSERT_FALSE(packetHasBpm(packet));
  TEST_ASSERT_TRUE(packetHasBpmLost(packet));
}

void test_bpm_lost_telemetry_waits_for_alert_flag() {
  TEST_ASSERT_FALSE(espNowShouldSendBpmLost(true, false, false));
  TEST_ASSERT_TRUE(espNowShouldSendBpmLost(true, false, true));
  TEST_ASSERT_FALSE(espNowShouldSendBpmLost(true, true, true));
  TEST_ASSERT_FALSE(espNowShouldSendBpmLost(false, false, true));
}

void test_packet_identifies_local_sender_for_broadcast_filtering() {
  EspNowRangePacket packet{};
  packet.senderNodeId = 1;

  TEST_ASSERT_TRUE(packetFromLocalNode(packet, 1));
  TEST_ASSERT_FALSE(packetFromLocalNode(packet, 0));
}
