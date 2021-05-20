#include "smol_test.h"

SMOL_TEST(force_success)
{
  SMOL_TEST_SUCCESS;
}

SMOL_TEST(float_equality_test)
{
  float a = 0.4f;
  float b = 0.4f;
  SMOL_TEST_EXPECT_FLOAT_EQ(a, b);
}

SMOL_TEST(float_inequality_test)
{
  float a = 0.45f;
  float b = 0.41f;
  SMOL_TEST_EXPECT_FLOAT_NEQ(a, b);
}

SMOL_TEST(equality_test)
{
  int a = 42;
  int b = 42;
  SMOL_TEST_EXPECT_FLOAT_EQ(a, b);
}

SMOL_TEST(inequality_test)
{
  int a = 42;
  int b = 1;
  SMOL_TEST_EXPECT_NEQ(a, b);
}

SMOL_TEST(greater_than_test)
{
  int a = 42;
  int b = 1;
  SMOL_TEST_EXPECT_GT(a, b);
}

SMOL_TEST(less_than_test)
{
  int a = 100;
  int b = 200;
  SMOL_TEST_EXPECT_LT(a, b);
}

//This tests intentionally fails
//SMOL_TEST(force_fail)
//{
//  SMOL_TEST_FAIL;
//}

