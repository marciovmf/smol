#ifndef SMOL_TEST_H
#define SMOL_TEST_H

#include <string>
#include <vector>
#include <limits>
#include <cmath>
#include <thread>

namespace smol
{
  struct Test;
  struct TestSuite;
  typedef void (*SmolTestFuncPtr)(smol::TestSuite&, smol::Test&);

  struct Test
  {
    enum State
    {
      NOT_RUN,
      FAILED,
      SUCCESS,
      RUNNING
    };

    State state;
    SmolTestFuncPtr func;
    std::string errorMsg;
    bool passed;
    bool abort;
    std::string name;

    Test(SmolTestFuncPtr func, const char*);
  };

  struct TestSuite
  {
    std::vector<Test> tests;
    TestSuite();
    static TestSuite& get();
    private:
    static TestSuite* self;
  };
}

#ifndef SMOL_TEST_EPSILON
#define SMOL_TEST_EPSILON std::numeric_limits<float>::epsilon()
#endif

#define SMOL_TEST_FAIL test.state = smol::Test::State::FAILED
#define SMOL_TEST_SUCCESS test.state = smol::Test::State::SUCCESS
#define SMOL_TEST_ERROR_MSG( msg )                    \
  test.errorMsg = std::string( msg )                  \
  + std::string( "\t@ " )                             \
  + std::string( __FILE__ )                           \
  + std::string( ":" )                                \
  + std::to_string( __LINE__ )                        \
  + std::string( "\n" )                               \

#define SMOL_TEST_EXPECT_FLOAT_EQ( x, y )             \
{                                                     \
  float epsilon = SMOL_TEST_EPSILON;                  \
  bool almostEqual = std::fabs(x - y) <= epsilon;     \
  if( !almostEqual )                                  \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(std::to_string((float)x) +    \
        " != " + std::to_string(y));                  \
  }                                                   \
}

#define SMOL_TEST_EXPECT_FLOAT_NEQ( x, y )            \
{                                                     \
  float epsilon = SMOL_TEST_EPSILON;                  \
  bool almostEqual = std::fabs(x - y) <= epsilon;     \
  if( almostEqual )                                   \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(std::to_string((float)x) +    \
        " ~= " + std::to_string(y));                  \
  }                                                   \
}

#define SMOL_TEST_EXPECT_EQ( x, y )                   \
{                                                     \
  if( ( x ) != ( y ) )                                \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(#x " != " #y);                \
  }                                                   \
}

#define SMOL_TEST_EXPECT_NEQ( x, y )                  \
{                                                     \
  if( ( x ) == ( y ) )                                \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(#x " == " #y);                \
  }                                                   \
}

#define SMOL_TEST_EXPECT_GT( x, y )                   \
{                                                     \
  if( ( x ) <= ( y ) )                                \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(#x " <= " #y);                \
  }                                                   \
}

#define SMOL_TEST_EXPECT_LT( x, y )                   \
{                                                     \
  if( ( x ) >= ( y ) )                                \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(#x " >= " #y);                \
  }                                                   \
}

#define SMOL_TEST_EXPECT_GE( x, y )                   \
{                                                     \
  if( ( x ) < ( y ) )                                 \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(#x " < " #y);                 \
  }                                                   \
}

#define SMOL_TEST_EXPECT_LE( x, y )                   \
{                                                     \
  if( ( x ) > ( y ) )                                 \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG(#x " > " #y);                 \
  }                                                   \
}

#define SMOL_TEST_EXPECT_TRUE( expr )                 \
{                                                     \
  if( !(expr) )                                       \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG("'" #expr "' is FALSE");      \
  }                                                   \
}

#define SMOL_TEST_EXPECT_FALSE( expr )                \
{                                                     \
  if( (expr) )                                        \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG("'" #expr "' is TRUE");       \
  }                                                   \
}

#define SMOL_TEST_EXPECT_NOT_NULL( expr )             \
{                                                     \
  if( (expr) == nullptr )                             \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG("'" #expr "' is nullptr");    \
  }                                                   \
}

#define SMOL_TEST_EXPECT_NULL( expr )                 \
{                                                     \
  if( (expr) != nullptr )                             \
  {                                                   \
    SMOL_TEST_FAIL;                                   \
    SMOL_TEST_ERROR_MSG("'" #expr "' is not nullptr");\
  }                                                   \
}
#define SMOL_TEST_WAIT_MILLISECONDS(num) std::this_thread::sleep_for(std::chrono::milliseconds((long long)(num)))

#define COMBINEHELPER(X,Y) X##Y
#define COMBINE(X,Y) COMBINEHELPER(X,Y)
#define __SMOL_TEST_ADD(testName) \
  int COMBINE(__smol_test_dummy,__LINE__) = (smol::TestSuite::get().tests.emplace_back(smol_test_##testName, #testName), 0)

#define SMOL_TEST(testName)\
  void smol_test_##testName(smol::TestSuite& testSuite, smol::Test& test);\
  __SMOL_TEST_ADD(testName);\
  void smol_test_##testName(smol::TestSuite& testSuite, smol::Test& test)

#endif  // SMOL_TEST_H
