
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
