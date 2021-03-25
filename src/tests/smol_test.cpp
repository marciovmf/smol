#include "smol_test.h"
#include <iostream>
#include <string>
#include <utility>

namespace smol
{
  TestSuite* TestSuite::self = nullptr;

  TestSuite::TestSuite(){}

  TestSuite& TestSuite::get()
  {
    if (TestSuite::self == nullptr)
      TestSuite::self = new TestSuite();

    return *TestSuite::self;
  }

  Test::Test(SmolTestFuncPtr func, const char* name):
    func(func), state(State::NOT_RUN), passed(false), abort(false), name(name)
  {
  }

  int TestSuite::runAllTests()
  {
    failCount = 0;
    int numTestesExecuted = 0;

    TestSuite& testSuite = TestSuite::get();
    for (smol::Test& i : tests)
    {
      std::cerr << "\t[WAIT]\t" << i.name; 
      i.state = Test::State::RUNNING;
      i.func(testSuite, i);
      numTestesExecuted++;

      if (i.state == Test::State::FAILED)
      {
        failCount++;

        std::cerr << "\r\t[FAIL]\t" << i.name << std::endl;
        if (i.errorMsg.length())
          std::cerr << "\t\t" << i.errorMsg << std::endl;

        if (i.abort)
        {
          std::cerr << "Aborting tests due to last error" << std::endl;
          break;
        }
      }
      else
      {
        std::cerr << "\r\t[PASS]\t" << i.name << std::endl;
      }
    }

    std::cerr << std::endl
      << "\tSucceeded tests: " << numTestesExecuted - failCount << "/" << numTestesExecuted << std::endl;

 
    return failCount;
  }
}

int main(int argc, char** argv)
{
  return smol::TestSuite::get().runAllTests();
}
