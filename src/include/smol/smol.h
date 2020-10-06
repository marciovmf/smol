#ifndef SMOL_H
#define SMOL_H

#include "smol_version.h"
// easier to check platform independent debug flag
#if defined(DEBUG) || defined(_DEBUG)
  #define SMOL_DEBUG
#endif


#ifdef SMOL_DEBUG
#include <stdio.h>
#define LOG(prefix, msg) printf("[%s] %s\n%s:%d\n", (prefix), (msg), __FILE__, __LINE__)
#define LOGINFO(msg) LOG("INFO", (msg))
#define LOGWARNING(msg) LOG("WARNING", (msg))
#define LOGERROR(msg) LOG("ERROR", (msg))
#else
#define LOGINFO(msg)
#define LOGWARNING(msg)
#define LOGERROR(msg)
#endif//SMOL_DEBUG


#endif //SMOL_H
