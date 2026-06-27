#include <unity.h>

#include "../src/CompassSensor.h"

void testCompassHeadingNormalizesCardinalDirections() {
  CompassCalibration calibration;

  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 0.0f, compassHeadingFromRaw(100, 0, calibration));
  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 90.0f, compassHeadingFromRaw(0, 100, calibration));
  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 180.0f, compassHeadingFromRaw(-100, 0, calibration));
  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 270.0f, compassHeadingFromRaw(0, -100, calibration));
}

void testCompassHeadingAppliesHardIronOffsetAndScale() {
  CompassCalibration calibration;
  calibration.magOffsetX = 10.0f;
  calibration.magOffsetY = -20.0f;
  calibration.magScaleX = 2.0f;
  calibration.magScaleY = 1.0f;

  TEST_ASSERT_FLOAT_WITHIN(
      0.001f, 26.565f, compassHeadingFromRaw(60, 30, calibration));
}

void testCompassDiagnosticStateTracksSampleAge() {
  CompassDiagnosticState state;
  state.lastUpdateMs = 1000;
  state.sampleCount = 3;

  updateCompassAge(state, 1250);

  TEST_ASSERT_EQUAL_UINT32(250, state.lastUpdateAgeMs);
  TEST_ASSERT_EQUAL_UINT32(3, state.sampleCount);
}

void testCompassHealthLabelUsesOkFailContract() {
  CompassDiagnosticState state;
  state.initialized = true;
  state.readOk = true;
  state.magAvailable = true;
  state.magReadOk = true;
  state.magDataOk = true;
  state.accelAvailable = true;
  state.accelReadOk = true;
  state.accelDataOk = true;

  TEST_ASSERT_EQUAL_STRING("OK", compassHealthLabel(state));

  state.accelAvailable = false;
  TEST_ASSERT_EQUAL_STRING("PARTIAL", compassHealthLabel(state));

  state.readOk = false;
  TEST_ASSERT_EQUAL_STRING("FAIL", compassHealthLabel(state));
}

void testCompassModeLabelsSeparateMagOnlyFromTiltCompensated() {
  CompassDiagnosticState state;
  state.initialized = true;
  state.magAvailable = true;
  state.readOk = true;
  state.magReadOk = true;
  state.magDataOk = true;
  state.accelAvailable = false;
  state.magAddress = kGy511MagAddress;
  state.compassMode = CompassMode::MagOnly;
  state.headingMode = CompassHeadingMode::MagOnly;

  TEST_ASSERT_EQUAL_STRING("OK", compassMagLabel(state));
  TEST_ASSERT_EQUAL_STRING("NONE", compassAccelLabel(state));
  TEST_ASSERT_EQUAL_STRING("0x1E", compassMagAddressLabel(state));
  TEST_ASSERT_EQUAL_STRING("NONE", compassAccelAddressLabel(state));
  TEST_ASSERT_EQUAL_STRING("MAG_ONLY", compassHeadingModeLabel(state));

  state.accelAvailable = true;
  state.accelReadOk = true;
  state.accelDataOk = true;
  state.accelAddress = kGy511FallbackAccelAddress;
  state.compassMode = CompassMode::Split6Dof;
  state.headingMode = CompassHeadingMode::TiltCompensated;
  TEST_ASSERT_EQUAL_STRING("OK", compassAccelLabel(state));
  TEST_ASSERT_EQUAL_STRING("0x18", compassAccelAddressLabel(state));
  TEST_ASSERT_EQUAL_STRING("TILT_COMPENSATED", compassHeadingModeLabel(state));
}

void testCompassRejectsRepeatedZeroMagnetometerSamples() {
  CompassDiagnosticState state;
  state.initialized = true;
  state.readOk = true;
  state.status = Gy511Status::Ok;
  state.magX = 0;
  state.magY = 0;
  state.magZ = 0;

  TEST_ASSERT_TRUE(updateCompassMagHealth(state));
  TEST_ASSERT_TRUE(updateCompassMagHealth(state));
  TEST_ASSERT_FALSE(updateCompassMagHealth(state));

  TEST_ASSERT_FALSE(state.readOk);
  TEST_ASSERT_EQUAL(Gy511Status::MagReadError, state.status);
  TEST_ASSERT_EQUAL_UINT8(3, state.zeroMagReadCount);
  TEST_ASSERT_EQUAL(CompassIssue::MagRawZero, state.lastIssue);
  TEST_ASSERT_EQUAL_STRING("MAG_RAW_ZERO", compassIssueLabel(state));
}

void testCompassDiagnosticIssueLabelsSeparateLockReadAndRawFailures() {
  CompassDiagnosticState state;

  state.lastIssue = CompassIssue::None;
  TEST_ASSERT_EQUAL_STRING("OK", compassIssueLabel(state));

  state.lastIssue = CompassIssue::LockTimeout;
  TEST_ASSERT_EQUAL_STRING("LOCK_TIMEOUT", compassIssueLabel(state));

  state.lastIssue = CompassIssue::MagReadError;
  TEST_ASSERT_EQUAL_STRING("MAG_READ_ERR", compassIssueLabel(state));

  state.lastIssue = CompassIssue::MagRawZero;
  TEST_ASSERT_EQUAL_STRING("MAG_RAW_ZERO", compassIssueLabel(state));
}

void testCompassMagRegisterDumpLabelsHmcCompatibleDevice() {
  CompassDiagnosticState state;
  state.magAvailable = true;
  state.magRegIdA = 0x48;
  state.magRegIdB = 0x34;
  state.magRegIdC = 0x33;
  state.magRegStatus = 0x01;
  state.magRegData[0] = 0x00;
  state.magRegData[1] = 0x64;
  state.magRegData[2] = 0x00;
  state.magRegData[3] = 0x08;
  state.magRegData[4] = 0xFF;
  state.magRegData[5] = 0x9C;

  TEST_ASSERT_EQUAL(CompassMagDriver::HmcCompatible0x1E, compassMagDriver(state));
  TEST_ASSERT_EQUAL_STRING("HMC_0x1E", compassMagDriverLabel(state));
}

void testCompassDetectsLsm303dAtShared0x1eAddress() {
  CompassDiagnosticState state;
  state.initialized = true;
  state.magAvailable = true;
  state.accelAvailable = true;
  state.magAddress = kGy511MagAddress;
  state.accelAddress = kGy511MagAddress;
  state.magWhoAmI = 0x49;
  state.readOk = true;
  state.magReadOk = true;
  state.accelReadOk = true;
  state.magDataOk = true;
  state.accelDataOk = true;
  state.headingDeg = 302.0f;
  state.magDriver = CompassMagDriver::Lsm303d;
  state.accelDriver = CompassAccelDriver::Lsm303dInternal;
  state.compassMode = CompassMode::Lsm303d6Dof;
  state.headingMode = CompassHeadingMode::MagOnly;

  TEST_ASSERT_EQUAL(CompassMagDriver::Lsm303d, compassMagDriver(state));
  TEST_ASSERT_EQUAL_STRING("LSM303D", compassMagDriverLabel(state));
  TEST_ASSERT_EQUAL_STRING("LSM303D_INTERNAL", compassAccelDriverLabel(state));
  TEST_ASSERT_EQUAL_STRING("LSM303D_6DOF", compassModeLabel(state));
  TEST_ASSERT_EQUAL_STRING("MAG_ONLY", compassHeadingModeLabel(state));
  TEST_ASSERT_EQUAL_STRING("OK", compassHealthLabel(state));
  TEST_ASSERT_EQUAL_STRING("0x1E", compassAccelAddressLabel(state));
}

void testCompassReportsPartialWhenOnlyMagHeadingIsUsable() {
  CompassDiagnosticState state;
  state.initialized = true;
  state.magAvailable = true;
  state.accelAvailable = true;
  state.magAddress = kGy511MagAddress;
  state.accelAddress = kGy511MagAddress;
  state.magWhoAmI = 0x49;
  state.readOk = true;
  state.magReadOk = true;
  state.accelReadOk = false;
  state.magDataOk = true;
  state.accelDataOk = false;
  state.headingDeg = 41.5f;
  state.magDriver = CompassMagDriver::Lsm303d;
  state.accelDriver = CompassAccelDriver::Lsm303dInternal;
  state.compassMode = CompassMode::Lsm303d6Dof;
  state.headingMode = CompassHeadingMode::MagOnly;

  TEST_ASSERT_TRUE(compassHeadingValid(state));
  TEST_ASSERT_EQUAL_STRING("PARTIAL", compassHealthLabel(state));
  TEST_ASSERT_EQUAL_STRING("FAIL", compassAccelLabel(state));
  TEST_ASSERT_EQUAL_STRING("MAG_ONLY", compassHeadingModeLabel(state));
}
