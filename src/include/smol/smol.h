#ifndef SMOL_H
#define SMOL_H

#include "smol_version.h"

// easier to check platform independent debug flag
#if defined(DEBUG) || defined(_DEBUG)
  #define SMOL_DEBUG
#endif

#ifdef _WIN32
  #define SMOL_PLATFORM_WINDOWS
 #else
  #define SMOL_PLATFORM_UNKNOWN
  #error "Unsuported platform"
#endif

#endif //SMOL_H
