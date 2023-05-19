#include "smol_test.h"
#include <smol/smol_handle_list.h>

struct Foo
{
  int x;
  float y;
  Foo(int pX, float pY):x(pX), y(pY) { }
};

struct Bar
{
  std::string name;
  Bar(std::string name): name(name) { }
};

SMOL_TEST(arrow_operator)
{
  smol::HandleList<Foo> hListFoo(8);
  smol::HandleList<Bar> hListBar(3);

  smol::Handle<Foo> hFoo = hListFoo.add(Foo(75, 4.2f));
  smol::Handle<Bar> hBar = hListBar.add(Bar("Wako"));
  hListBar.add(Bar("Yako"));

  SMOL_TEST_EXPECT_TRUE(hBar->name == "Wako");
  SMOL_TEST_EXPECT_EQ(hFoo->x, 75);
  SMOL_TEST_EXPECT_FLOAT_EQ(hFoo->y, 4.2f);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup)
{
  smol::HandleList<Foo> hList(8);
  hList.add(Foo(75, 4.2f));
  hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  hList.add(Foo(485, 24.4f));

  //SMOL_TEST_EXPECT_NOT_NULL(foo);
  SMOL_TEST_EXPECT_EQ(hFoo->x, 22);
  SMOL_TEST_EXPECT_FLOAT_EQ(hFoo->y, 56624.21f);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup_wtih_holes_before)
{
  smol::HandleList<Foo> hList(8);
  smol::Handle<Foo> hDelete = hList.add(Foo(75, 4.2f));
  hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  hList.add(Foo(485, 24.4f));

  hList.remove(hDelete);

  SMOL_TEST_EXPECT_EQ(hFoo->x, 22);
  SMOL_TEST_EXPECT_FLOAT_EQ(hFoo->y, 56624.21f);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup_wtih_holes_after)
{
  smol::HandleList<Foo> hList(8);
  hList.add(Foo(75, 4.2f));
  hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  smol::Handle<Foo> hDelete = hList.add(Foo(485, 24.4f));

  hList.remove(hDelete);
  Foo* foo = hList.lookup(hFoo);
  SMOL_TEST_EXPECT_NOT_NULL(foo);
  SMOL_TEST_EXPECT_EQ(hFoo->x, 22);
  SMOL_TEST_EXPECT_FLOAT_EQ(hFoo->y, 56624.21f);
  smol::Handle<Foo>::registerList(nullptr);
}


SMOL_TEST(lookup_wtih_holes_around)
{
  smol::HandleList<Foo> hList(8);
  hList.add(Foo(75, 4.2f));
  smol::Handle<Foo> hDelete1 = hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  smol::Handle<Foo> hDelete2 = hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  smol::Handle<Foo> hDelete3 = hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  smol::Handle<Foo> hDelete4 = hList.add(Foo(485, 24.4f));

  hList.remove(hDelete1);
  hList.remove(hDelete2);
  hList.remove(hDelete3);
  hList.remove(hDelete4);
  Foo* foo = hList.lookup(hFoo);

  SMOL_TEST_EXPECT_NOT_NULL(foo);
  SMOL_TEST_EXPECT_EQ(foo->x, 22);
  SMOL_TEST_EXPECT_FLOAT_EQ(foo->y, 56624.21f);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup_wtih_holes_around_different_order_1)
{
  smol::HandleList<Foo> hList(8);
  hList.add(Foo(75, 4.2f));
  smol::Handle<Foo> hDelete1 = hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  smol::Handle<Foo> hDelete2 = hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  smol::Handle<Foo> hDelete3 = hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  smol::Handle<Foo> hDelete4 = hList.add(Foo(485, 24.4f));

  hList.remove(hDelete4);
  hList.remove(hDelete3);
  hList.remove(hDelete2);
  hList.remove(hDelete1);
  Foo* foo = hList.lookup(hFoo);

  SMOL_TEST_EXPECT_NOT_NULL(foo);
  SMOL_TEST_EXPECT_EQ(foo->x, 22);
  SMOL_TEST_EXPECT_FLOAT_EQ(foo->y, 56624.21f);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup_wtih_holes_around_different_order_2)
{
  smol::HandleList<Foo> hList(8);
  hList.add(Foo(75, 4.2f));
  smol::Handle<Foo> hDelete1 = hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  smol::Handle<Foo> hDelete2 = hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  smol::Handle<Foo> hDelete3 = hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  smol::Handle<Foo> hDelete4 = hList.add(Foo(485, 24.4f));

  hList.remove(hDelete2);
  hList.remove(hDelete4);
  hList.remove(hDelete3);
  hList.remove(hDelete1);
  Foo* foo = hList.lookup(hFoo);

  SMOL_TEST_EXPECT_NOT_NULL(foo);
  SMOL_TEST_EXPECT_EQ(foo->x, 22);
  SMOL_TEST_EXPECT_FLOAT_EQ(foo->y, 56624.21f);

  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup_removed)
{
  smol::HandleList<Foo> hList(8);
  smol::Handle<Foo> hFoo = hList.add(Foo(75, 4.2f));
  hList.remove(hFoo);
  Foo* foo = hList.lookup(hFoo);
  SMOL_TEST_EXPECT_NULL(foo);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(lookup_outaded_handle)
{
  smol::HandleList<Foo> hList(8);
  hList.add(Foo(75, 4.2f));
  hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> hFoo = hList.add(Foo(22, 56624.21f));
  hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  hList.add(Foo(485, 24.4f));
  hList.remove(hFoo);

  smol::Handle<Foo> hNewFoo = hList.add(Foo(77, 324.47f));
  SMOL_TEST_EXPECT_FALSE(hNewFoo == hFoo);
  SMOL_TEST_EXPECT_TRUE(hNewFoo != hFoo);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(add_expand)
{
  const int initialResourceCount = 8;
  smol::HandleList<Foo> hList(initialResourceCount);
  hList.add(Foo(75, 4.2f));
  hList.add(Foo(15, 14.2f));
  hList.add(Foo(25, 220.08f));
  hList.add(Foo(53, 2224.2f));
  hList.add(Foo(22, 56624.21f));
  hList.add(Foo(984, 24.8f));
  hList.add(Foo(475, 24.5f));
  hList.add(Foo(485, 24.4f));
  smol::Handle<Foo> hOutOfBounds = hList.add(Foo(44, 55.5f));
  Foo* foo = hList.lookup(hOutOfBounds);

  SMOL_TEST_EXPECT_NOT_NULL(foo);
  SMOL_TEST_EXPECT_EQ(foo->x, 44);
  SMOL_TEST_EXPECT_FLOAT_EQ(foo->y, 55.5f);
  SMOL_TEST_EXPECT_EQ(hList.count(), initialResourceCount + 1);
  smol::Handle<Foo>::registerList(nullptr);
}

SMOL_TEST(reset_and_lookup)
{
  smol::HandleList<Foo> hList(8);
  smol::Handle<Foo> h1 = hList.add(Foo(75, 4.2f));
  smol::Handle<Foo> h2 = hList.add(Foo(15, 14.2f));
  smol::Handle<Foo> h3 = hList.add(Foo(25, 220.08f));
  smol::Handle<Foo> h4 = hList.add(Foo(53, 2224.2f));
  smol::Handle<Foo> h5 = hList.add(Foo(22, 56624.21f));
  smol::Handle<Foo> h6 = hList.add(Foo(984, 24.8f));
  smol::Handle<Foo> h7 = hList.add(Foo(475, 24.5f));
  smol::Handle<Foo> h8 = hList.add(Foo(485, 24.4f));
  hList.reset();

  SMOL_TEST_EXPECT_EQ(hList.count(), 0);
  SMOL_TEST_EXPECT_NULL(hList.lookup(h1));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h2));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h3));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h4));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h5));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h6));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h7));
  SMOL_TEST_EXPECT_NULL(hList.lookup(h8));
  smol::Handle<Foo>::registerList(nullptr);
}
