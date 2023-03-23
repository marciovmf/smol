#include "smol_test.h"
#include <smol/smol_arena.h>

SMOL_TEST(initialization)
{
  const unsigned long capacity = MEGABYTE(1);
  smol::Arena arena(capacity);
  SMOL_TEST_EXPECT_EQ(arena.getCapacity(), capacity);
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), 0);
  SMOL_TEST_EXPECT_NEQ(arena.getData(), nullptr);
}

SMOL_TEST(late_initialization)
{
  const unsigned long capacity = MEGABYTE(1);
  smol::Arena arena;
  arena.initialize(capacity);
  SMOL_TEST_EXPECT_EQ(arena.getCapacity(), capacity);
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), 0);
  SMOL_TEST_EXPECT_NEQ(arena.getData(), nullptr);
}

SMOL_TEST(used_amount)
{
  const unsigned long capacity = MEGABYTE(1);
  smol::Arena arena(capacity);
  arena.pushSize(KILOBYTE(256));
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), KILOBYTE(256));
  arena.pushSize(KILOBYTE(256));
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), KILOBYTE(512));
  arena.pushSize(KILOBYTE(512));
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), KILOBYTE(1024));
}

SMOL_TEST(used_amount_reset)
{
  const unsigned long capacity = MEGABYTE(1);
  smol::Arena arena(capacity);
  arena.pushSize(KILOBYTE(256));
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), KILOBYTE(256));
  arena.pushSize(KILOBYTE(256));
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), KILOBYTE(512));
  arena.pushSize(KILOBYTE(512));
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), KILOBYTE(1024));
  arena.reset();
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), 0);
}

SMOL_TEST(expand)
{
  const unsigned long capacity = MEGABYTE(1);
  const unsigned long tooLargePushSize = capacity * 2;

  smol::Arena arena(capacity);
  SMOL_TEST_EXPECT_EQ(arena.getCapacity(), capacity);

  arena.pushSize(tooLargePushSize);
  SMOL_TEST_EXPECT_EQ(arena.getUsed(), tooLargePushSize);
  SMOL_TEST_EXPECT_GE(arena.getCapacity(), tooLargePushSize);
}
