#include <smol/smol_log.h>
#include <smol/smol_engine.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <io.h>

#ifdef SMOL_PLATFORM_WINDOWS
#include <windows.h>
  struct Win32ConsoleHelper
  {
    enum
    {
      RED_TEXT = 12,
      YELLOW_TEXT = 14,
      WHITE_TEXT = 15
    };

    HANDLE hConsole;
    int defaultAttributes;
    Win32ConsoleHelper()
    {
      hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
      CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
      GetConsoleScreenBufferInfo(hConsole, &consoleInfo);
      defaultAttributes = consoleInfo.wAttributes;
    }

    void setConsoleTextAttribute(int attribute)
    {
      SetConsoleTextAttribute(hConsole, attribute);
    }

    void resetConsoleDefaultAttributes()
    {
      SetConsoleTextAttribute(hConsole, defaultAttributes);
    }
  };
  static Win32ConsoleHelper win32ConsoleHelper;

#define dup(a) _dup((a))
#define dup2(a, b) _dup2((a),(b))
#define fileno(a) _fileno((a))
#define close(a) _close((a))
#endif

namespace smol
{
  static FILE* globalStream = 0;
  static int globalStdoutCopy = 0;

  void vprint(smol::Log::LogType type, const char* context, const char* format, va_list argList)
  {
    int current = (int) Log::verbosity(-1);
    if ((current & (int)type) != type)
      return;

    time_t longTime;
    struct tm localTime;
    time(&longTime);

#ifdef SMOL_PLATFORM_WINDOWS
    // Convert to local time.
    localtime_s(&localTime, &longTime);
#else
    localtime_r(&longTime, &localTime);
#endif
    const char* tag;
    bool shouldFlush = true;
    switch (type)
    {
      case Log::LogType::LOG_FATAL:
        tag = (const char*) "FATAL";
        break;

      case Log::LogType::LOG_ERROR:
        tag = (const char*) "ERROR";
        break;

      case Log::LogType::LOG_WARNING:
        tag = (const char*) "WARNING";
        shouldFlush = false;
        break;

      case Log::LogType::LOG_INFO:
      default:
        tag = (const char*) "INFO";
        shouldFlush = false;
        break;
    }

#ifdef SMOL_PLATFORM_WINDOWS
    bool loggingToStdout = globalStream == 0;
    if (loggingToStdout)
    {
      int color;
      if (type == Log::LogType::LOG_INFO)
        color = Win32ConsoleHelper::WHITE_TEXT;
      else if (type == Log::LogType::LOG_WARNING)
        color = Win32ConsoleHelper::YELLOW_TEXT;
      else
        color = Win32ConsoleHelper::RED_TEXT;
      win32ConsoleHelper.setConsoleTextAttribute(color);
    }
#endif

    fprintf(stdout, "\n%02d/%02d/%02d %02d:%02d:%02d - %-8s:\t",
        localTime.tm_year + 1900,
        localTime.tm_mon + 1,
        localTime.tm_mday,
        localTime.tm_hour,
        localTime.tm_min, 
        localTime.tm_sec,
        tag);

#ifdef SMOL_PLATFORM_WINDOWS
    if (loggingToStdout)
      win32ConsoleHelper.resetConsoleDefaultAttributes();
#endif
    vfprintf(stdout, format, argList);
    
    if(context) printf("\n\t%s", context);

    if(shouldFlush) fflush(stdout);
  }

  Log::LogType Log::verbosity(int maxLogLevel)
  {
    static int logLevel = Log::LogType::LOG_ALL;

    if (maxLogLevel != -1)
      logLevel = maxLogLevel;

    return (Log::LogType) logLevel;
  }

  void Log::toStdout()
  {
    if (globalStream)
    {
      fflush(stdout);
      dup2(globalStdoutCopy, fileno(stdout));
      close(globalStdoutCopy);
      globalStream = 0;
    }
  }

  void Log::toFile(const char* fileName)
  {
    if (globalStream)
    {
      toStdout();
    }

    fflush(stdout);
    bool error = false;
    globalStdoutCopy = dup(1);

    #ifdef SMOL_PLATFORM_WINDOWS
        error = freopen_s(&globalStream, fileName, "a+", stdout) != 0;
    #else
        globalStream = freopen(fileName, "a+", stdout);
        error = (fd == 0 ); 
    #endif

    if (!globalStream)
    {
      fatal("Could not redirect stdout to log file '%s'", fileName);
      return;
    }
  }

  void Log::print(smol::Log::LogType type, const char* context, const char* format, ...)
  {
    va_list argptr;
    va_start(argptr, format);
    vprint(type, context, format, argptr);
    va_end(argptr);
  }

  void Log::error(const char* format, ...)
  {
    va_list argptr;
    va_start(argptr, format);
    vprint(Log::LogType::LOG_ERROR, nullptr, format, argptr);
    va_end(argptr);
  }

  void Log::info(const char* format, ...)
  {
    va_list argptr;
    va_start(argptr, format);
    vprint(Log::LogType::LOG_INFO, nullptr, format, argptr);
    va_end(argptr);
  }

  void Log::warning(const char* format, ...)
  {
    va_list argptr;
    va_start(argptr, format);
    vprint(Log::LogType::LOG_WARNING, nullptr, format, argptr);
    va_end(argptr);
  }

  void Log::fatal(const char* format, ...)
  {
    va_list argptr;
    va_start(argptr, format);
    vprint(Log::LogType::LOG_FATAL, nullptr, format, argptr);
    va_end(argptr);
  }
}
