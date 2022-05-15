#ifndef SMOL_H
#define SMOL_H

#include "smol_version.h"

// easier to check platform independent debug flag
#if defined(DEBUG) || defined(_DEBUG)
  #define SMOL_DEBUG
#endif

#ifdef _WIN32
  #define SMOL_PLATFORM_WINDOWS
  #define _CRT_SECURE_NO_WARNINGS
#else
  #define SMOL_PLATFORM_UNKNOWN
  #error "Unsuported platform"
#endif // _WIN32

#include <stdint.h>
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef unsigned char uchar;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

#endif //SMOL_H
