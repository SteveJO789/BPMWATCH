#include <unity.h>

#include "../src/SosButton.h"

void testSosButtonIgnoresShortPress() {
  SosButtonState state;
  SosButtonConfig config;

  updateSosButton({1000, true}, state, config);
  updateSosButton({1060, true}, state, config);
  updateSosButton({1200, false}, state, config);
  updateSosButton({1260, false}, state, config);

  TEST_ASSERT_FALSE(state.sosActive);
  TEST_ASSERT_EQUAL_UINT32(0, state.sosSeq);
  TEST_ASSERT_EQUAL(SosButtonEvent::ShortPress, state.lastEvent);
}

void testSosButtonLongPressTogglesSosAndIncrementsSequence() {
  SosButtonState state;
  SosButtonConfig config;

  updateSosButton({1000, true}, state, config);
  updateSosButton({1060, true}, state, config);
  updateSosButton({2060, true}, state, config);

  TEST_ASSERT_TRUE(state.sosActive);
  TEST_ASSERT_EQUAL_UINT32(1, state.sosSeq);
  TEST_ASSERT_EQUAL(SosButtonEvent::LongPressToggle, state.lastEvent);
  TEST_ASSERT_EQUAL_UINT32(1000, state.lastEventDurationMs);

  updateSosButton({2200, false}, state, config);
  updateSosButton({2260, false}, state, config);
  updateSosButton({3000, true}, state, config);
  updateSosButton({3060, true}, state, config);
  updateSosButton({4060, true}, state, config);

  TEST_ASSERT_FALSE(state.sosActive);
  TEST_ASSERT_EQUAL_UINT32(2, state.sosSeq);
}

void testSosButtonDebouncesRawInput() {
  SosButtonState state;
  SosButtonConfig config;

  updateSosButton({1000, true}, state, config);
  updateSosButton({1020, false}, state, config);
  updateSosButton({1040, true}, state, config);

  TEST_ASSERT_FALSE(state.stablePressed);
  TEST_ASSERT_TRUE(state.rawPressed);

  updateSosButton({1095, true}, state, config);

  TEST_ASSERT_TRUE(state.stablePressed);
  TEST_ASSERT_EQUAL(SosButtonEvent::Pressed, state.lastEvent);
}

void testRemoteSosExpiresWhenPeerStopsSending() {
  RemoteSosState remote;
  remote.active = true;
  remote.lastRxMs = 1000;

  TEST_ASSERT_TRUE(remoteSosVisible(remote, 6500));
  TEST_ASSERT_FALSE(remoteSosVisible(remote, 11001));
}
