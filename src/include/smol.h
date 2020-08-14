#ifndef SMOL_H
#define SMOL_H

#if defined(_WIN32) || defined(WIN64)
  #if SMOL_ENGINE
    SMOL_API _declspec(dllexport)
  #else
    SMOL_API _declspec(dllimport)
  #endif // SMOL_ENGINE
#else
  #error Unsupported Platform
#endif //defined(_WIN32) || defined(WIN64)

#include <stdio.h>
#include <smol_types.h>

#endif //SMOL_H
