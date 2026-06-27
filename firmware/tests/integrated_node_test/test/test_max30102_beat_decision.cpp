#include <unity.h>

#include "../src/Max30102BeatDecision.h"

void testMax30102BeatDecisionSeedsFirstAcceptedBeat() {
  TEST_ASSERT_EQUAL(Max30102BeatDecisionSeed,
                    max30102BeatCandidateDecision(false, 0));
}

void testMax30102BeatDecisionReseedsStaleAcceptedClock() {
  TEST_ASSERT_EQUAL(Max30102BeatDecisionLongIntervalReseed,
                    max30102BeatCandidateDecision(true, 37110));
}

void testMax30102BeatDecisionRejectsShortAndRefractoryIntervals() {
  TEST_ASSERT_EQUAL(Max30102BeatDecisionRefractoryReject,
                    max30102BeatCandidateDecision(true, 180));
  TEST_ASSERT_EQUAL(Max30102BeatDecisionShortIntervalReject,
                    max30102BeatCandidateDecision(true, 380));
}

void testMax30102BeatDecisionAcceptsPhysiologicInterval() {
  TEST_ASSERT_EQUAL(Max30102BeatDecisionAccept,
                    max30102BeatCandidateDecision(true, 800));
}
