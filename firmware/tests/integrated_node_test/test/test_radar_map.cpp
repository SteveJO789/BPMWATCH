
#include <unity.h>

#include "../src/RadarMap.h"

void testRadarMapStartsWithPeerInFront() {
  RadarState state;
  updateRadarState({5.0f, true, 1000}, state);

  TEST_ASSERT_TRUE(state.hasAngle);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, state.peerAngleDeg);
  TEST_ASSERT_FLOAT_WITHIN(0.001f, 5.0f, state.peerDistanceM);
  TEST_ASSERT_FALSE(state.hidePeerDot);
}

void testRadarMapUsesFrontWhenDistanceDecreases() {
  RadarState state;
  updateRadarState({8.0f, true, 1000}, state);
  state.peerAngleDeg = 180.0f;

  updateRadarState({7.5f, true, 1200}, state);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, state.peerAngleDeg);
}

void testRadarMapUsesBackWhenDistanceIncreases() {
  RadarState state;
  updateRadarState({5.0f, true, 1000}, state);

  updateRadarState({5.5f, true, 1200}, state);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, 180.0f, state.peerAngleDeg);
}

void testRadarMapKeepsAngleWhenDistanceBarelyChanges() {
  RadarState state;
  updateRadarState({5.0f, true, 1000}, state);
  updateRadarState({5.5f, true, 1200}, state);

  updateRadarState({5.6f, true, 1400}, state);

  TEST_ASSERT_FLOAT_WITHIN(0.001f, 180.0f, state.peerAngleDeg);
}

void testRadarMapHidesDotAfterLinkTimeout() {
  RadarState state;
  updateRadarState({5.0f, true, 1000}, state);

  updateRadarState({0.0f, false, 4501}, state);

  TEST_ASSERT_TRUE(state.hidePeerDot);
}

void testRadarMapReportsWaitBeforeFirstRange() {
  RadarState state;

  TEST_ASSERT_EQUAL_STRING("WAIT", radarLinkStatusLabel(true, state));
}

void testRadarMapReportsLostOnlyAfterPreviousRangeTimesOut() {
  RadarState state;
  updateRadarState({5.0f, true, 1000}, state);
  updateRadarState({0.0f, false, 4501}, state);

  TEST_ASSERT_EQUAL_STRING("LOST", radarLinkStatusLabel(true, state));
}

void testRadarMapRotatesDemoAngleIntoNorthOrientedDisplay() {
  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 90.0f, northOrientedRadarAngleDeg(0.0f, 90.0f, true));
  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 90.0f, northOrientedRadarAngleDeg(180.0f, 270.0f, true));
}

void testRadarMapKeepsDemoAngleWhenHeadingInvalid() {
  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 180.0f, northOrientedRadarAngleDeg(180.0f, 270.0f, false));
}

void testRadarBpmLostPolicyRequiresUsableBpm() {
  TEST_ASSERT_TRUE(radarBpmLost(true, false, false, false, false));
  TEST_ASSERT_TRUE(radarBpmLost(true, true, false, false, false));
  TEST_ASSERT_TRUE(radarBpmLost(true, true, true, false, false));
  TEST_ASSERT_TRUE(radarBpmLost(true, true, true, false, true));
  TEST_ASSERT_FALSE(radarBpmLost(true, true, true, true, true));
  TEST_ASSERT_FALSE(radarBpmLost(false, false, false, false, false));
}

void testRadarBpmLostAlertWaitsFiveMinutesForNoFinger() {
  uint32_t invalidSinceMs = 0;
  uint32_t invalidAgeMs = 0;
  bool alert = false;

  updateRadarBpmLostAlert(true, false, 1000, invalidSinceMs,
                          invalidAgeMs, alert);
  TEST_ASSERT_EQUAL_UINT32(1000, invalidSinceMs);
  TEST_ASSERT_EQUAL_UINT32(0, invalidAgeMs);
  TEST_ASSERT_FALSE(alert);

  updateRadarBpmLostAlert(true, false, 1000 + kBpmLostAlertGraceMs - 1,
                          invalidSinceMs, invalidAgeMs, alert);
  TEST_ASSERT_FALSE(alert);

  updateRadarBpmLostAlert(true, false, 1000 + kBpmLostAlertGraceMs,
                          invalidSinceMs, invalidAgeMs, alert);
  TEST_ASSERT_EQUAL_UINT32(kBpmLostAlertGraceMs, invalidAgeMs);
  TEST_ASSERT_TRUE(alert);
}

void testRadarBpmLostAlertResetsWhenBpmReturns() {
  uint32_t invalidSinceMs = 1000;
  uint32_t invalidAgeMs = kBpmLostAlertGraceMs;
  bool alert = true;

  updateRadarBpmLostAlert(true, true, 1000 + kBpmLostAlertGraceMs + 1,
                          invalidSinceMs, invalidAgeMs, alert);

  TEST_ASSERT_EQUAL_UINT32(0, invalidSinceMs);
  TEST_ASSERT_EQUAL_UINT32(0, invalidAgeMs);
  TEST_ASSERT_FALSE(alert);
}

void testRadarNodeAlertUsesSosOrBpmLost() {
  TEST_ASSERT_TRUE(radarNodeAlert(true, false));
  TEST_ASSERT_TRUE(radarNodeAlert(false, true));
  TEST_ASSERT_FALSE(radarNodeAlert(false, false));
}

void testRemoteBpmLostExpiresWithSnapshot() {
  TEST_ASSERT_FALSE(remoteBpmLostVisible(true, 0, 1000));
  TEST_ASSERT_TRUE(remoteBpmLostVisible(true, 1000, 3500));
  TEST_ASSERT_FALSE(remoteBpmLostVisible(true, 1000, 5001));
  TEST_ASSERT_FALSE(remoteBpmLostVisible(false, 1000, 3500));
}
