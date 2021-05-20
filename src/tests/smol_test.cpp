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

  void TestSuite::printPassedStatus(std::string& msg)
  {
#ifdef _WIN32
    win32Console.setGreenTextColor();
#endif
      std::cerr << "\r\t[  OK  ]\t";
#ifdef _WIN32
    win32Console.setDefaultTextColor();
#endif
    std::cerr << msg << std::endl;
  }
  
  void TestSuite::printFailStatus(std::string& msg)
  {
#ifdef _WIN32
    win32Console.setRedTextColor();
#endif
    std::cerr << "\r\t[ FAIL ]\t";
#ifdef _WIN32
    win32Console.setDefaultTextColor();
#endif
    std::cerr << msg << std::endl;
  }

  void TestSuite::printWaitStatus(std::string& msg)
  {
#ifdef _WIN32
    win32Console.setYellowTextColor();
#endif
    std::cerr << "\t[ WAIT ]\t";
#ifdef _WIN32
    win32Console.setDefaultTextColor();
#endif
    std::cerr << msg; 
  }

  int TestSuite::runAllTests()
  {
    failCount = 0;
    int numTestesExecuted = 0;

    TestSuite& testSuite = TestSuite::get();
    for (smol::Test& i : tests)
    {
      //std::cerr << "\t[ WAIT ]\t" << i.name; 
      printWaitStatus(i.name);
      i.state = Test::State::RUNNING;
      i.func(testSuite, i);
      numTestesExecuted++;

      if (i.state == Test::State::FAILED)
      {
        failCount++;

        //std::cerr << "\r\t[ FAIL ]\t" << i.name << std::endl;
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
        //std::cerr << "\r\t[  OK  ]\t" << i.name << std::endl;
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
  return smol::TestSuite::get().runAllTests();
}
