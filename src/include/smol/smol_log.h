#ifndef SMOL_LOG
#define SMOL_LOG

#include <smol/smol_engine.h>
#include <stdarg.h>
#include <stdio.h>

#ifdef SMOL_DEBUG
#define STRNOEXPAND(a) ":"#a
#define STR(a) STRNOEXPAND(a)
#define SMOLCONTEXT STR(__FILE__) ## STR(__LINE__)
#define debugLogError(fmt, ...) smol::Log::print(smol::Log::LogType::LOG_ERROR, SMOLCONTEXT, fmt, __VA_ARGS__)
#define debugLogFatal(fmt, ...) smol::Log::print(smol::Log::LogType::LOG_FATAL, SMOLCONTEXT, fmt, __VA_ARGS__)
#define debugLogInfo(fmt, ...) smol::Log::print(smol::Log::LogType::LOG_INFO, nullptr, fmt, __VA_ARGS__)
#define debugLogWarning(fmt, ...) smol::Log::print(smol::Log::LogType::LOG_WARNING, SMOLCONTEXT, fmt, __VA_ARGS__)
#define SMOL_ASSERT(condition, fmt, ...) 
//do{ if (!(condition)) {smol::Log::info(fmt, __VA_ARGS__); printf("\n\t%s:%d", __FILE__, __LINE__);  *((int*)0) = 0;} } while(0)
#else
#define debugLogError(fmt, ...)
#define debugLogFatal(fmt, ...)
#define debugLogInfo(fmt, ...)
#define debugLogWarning(fmt, ...)
#define SMOL_ASSERT(condition, fmt, ...)
#endif


namespace smol
{
  struct SMOL_ENGINE_API Log
  {
    enum LogType
    {
      LOG_INFO = 1 << 1,
      LOG_WARNING = 1 << 2,
      LOG_ERROR = 1 << 3,
      LOG_FATAL = 1 << 4
    };

    static void toFile(const char* fileName);
    static void toStdout();
    static Log::LogType verbosity(Log::LogType maxLogLevel);
    static void error(const char* format, ...);
    static void info(const char* format, ...);
    static void warning(const char* format, ...);
    static void fatal(const char* format, ...);
    static void print(Log::LogType type, const char* context, const char* format, ...);
    static void vprint(Log::LogType type, const char* context, const char* format, va_list args);
  };
}

#endif  // SMOL_LOG
