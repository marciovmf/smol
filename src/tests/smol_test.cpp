#include "smol_test.h"
#include <iostream>
#include <string>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#endif

namespace smol
{
#if _WIN32
  struct Win32Console
  {
    HANDLE hConsole;
    int defaultColors;

    Win32Console()
    {
      CONSOLE_SCREEN_BUFFER_INFO csbi;
      hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      GetConsoleScreenBufferInfo(hConsole, &csbi);
      defaultColors = csbi.wAttributes;
    }

    void setRedTextColor() { SetConsoleTextAttribute(hConsole, 12); }
    void setGreenTextColor() { SetConsoleTextAttribute(hConsole, 10); }
    void setYellowTextColor() { SetConsoleTextAttribute(hConsole, 14); }
    void setDefaultTextColor() { SetConsoleTextAttribute(hConsole, defaultColors); }
  };

  static Win32Console win32Console;
#endif

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


  void printPassedStatus(std::string& msg)
  {
    std::cerr << "\r[";
#ifdef _WIN32
    win32Console.setGreenTextColor();
#endif
    std::cerr << "PASS";
#ifdef _WIN32
    win32Console.setDefaultTextColor();
#endif
    std::cerr << "] " << msg << std::endl;
  }

  void printFailStatus(std::string& msg)
  {
    std::cerr << "\r[";
#ifdef _WIN32
    win32Console.setRedTextColor();
#endif
    std::cerr << "FAIL";
#ifdef _WIN32
    win32Console.setDefaultTextColor();
#endif
    std::cerr << "] " << msg << std::endl;
  }

  void printWaitStatus(std::string& msg)
  {
    std::cerr << "\r[";
#ifdef _WIN32
    win32Console.setYellowTextColor();
#endif
    std::cerr << "WAIT";
#ifdef _WIN32
    win32Console.setDefaultTextColor();
#endif
    std::cerr << "] " << msg << std::flush;
  }

  int runTestSuite(TestSuite& testSuite)
  {
    int failCount = 0;
    int numTestesExecuted = 0;
    std::cerr << std::endl;

    for (smol::Test& i : testSuite.tests)
    {
      printWaitStatus(i.name);
      i.state = Test::State::RUNNING;
      i.func(testSuite, i);
      numTestesExecuted++;

      if (i.state == Test::State::FAILED)
      {
        failCount++;

        printFailStatus(i.name);
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
        printPassedStatus(i.name);
      }
    }

    std::cerr << std::endl
      << "\tSucceeded tests: " << numTestesExecuted - failCount << "/" << numTestesExecuted << std::endl;


    return failCount;
  }
}

int main(int argc, char** argv)
{
  return smol::runTestSuite(smol::TestSuite::get());
}
