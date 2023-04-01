#include "smol_test.h"
#include <smol/smol_mat4.h>

SMOL_TEST(identity)
{
  smol::Mat4 mat = smol::Mat4::initIdentity();

  for (int i = 0; i < 4; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      if (i == j)
      {
        SMOL_TEST_EXPECT_FLOAT_EQ(mat.e[i][j], 1.0f);
      }
      else
      {
        SMOL_TEST_EXPECT_FLOAT_EQ(mat.e[i][j], 0.0f);
      }
    }
  }
}

SMOL_TEST(identity_multiplication)
{
  smol::Mat4 mat;

  // just fill the matrix with some dummy values
  for (int i=0; i < 4; i++)
  {
    for (int j=0; j < 4; j++)
    {
      mat.e[i][j] = 1.0321f + (100.123f * (i + j));
    }
  }

  smol::Mat4 identity = smol::Mat4::initIdentity();
  smol::Mat4 result = smol::Mat4::mul(mat, identity);

  for (int i=0; i < 4; i++)
  {
    for (int j=0; j < 4; j++)
    {
      SMOL_TEST_EXPECT_FLOAT_EQ(result.e[i][j], mat.e[i][j]);
    }
  }
}
