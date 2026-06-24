#include <unity.h>

#include "../src/Gy511Status.h"

void testGy511StatusLabelsExposeFailingI2cSide() {
  TEST_ASSERT_EQUAL_STRING("INIT ERR", gy511StatusLabel(Gy511Status::InitError));
  TEST_ASSERT_EQUAL_STRING("ACC INIT", gy511StatusLabel(Gy511Status::AccelInitError));
  TEST_ASSERT_EQUAL_STRING("MAG INIT", gy511StatusLabel(Gy511Status::MagInitError));
  TEST_ASSERT_EQUAL_STRING("I2C INIT", gy511StatusLabel(Gy511Status::I2cInitError));
  TEST_ASSERT_EQUAL_STRING("MAG ONLY", gy511StatusLabel(Gy511Status::MagOnly));
  TEST_ASSERT_EQUAL_STRING("ACC ERR", gy511StatusLabel(Gy511Status::AccelReadError));
  TEST_ASSERT_EQUAL_STRING("MAG ERR", gy511StatusLabel(Gy511Status::MagReadError));
  TEST_ASSERT_EQUAL_STRING("I2C ERR", gy511StatusLabel(Gy511Status::I2cReadError));
  TEST_ASSERT_EQUAL_STRING("OK", gy511StatusLabel(Gy511Status::Ok));
}

void testGy511InitFailureStatusesArePreservedDuringSampling() {
  TEST_ASSERT_TRUE(gy511StatusIsInitFailure(Gy511Status::InitError));
  TEST_ASSERT_TRUE(gy511StatusIsInitFailure(Gy511Status::AccelInitError));
  TEST_ASSERT_TRUE(gy511StatusIsInitFailure(Gy511Status::MagInitError));
  TEST_ASSERT_TRUE(gy511StatusIsInitFailure(Gy511Status::I2cInitError));
  TEST_ASSERT_FALSE(gy511StatusIsInitFailure(Gy511Status::AccelReadError));
  TEST_ASSERT_FALSE(gy511StatusIsInitFailure(Gy511Status::Ok));
}
