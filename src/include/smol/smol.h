#ifndef SMOL_H
#define SMOL_H

#include "smol_version.h"
// easier to check platform independent debug flag
#if defined(DEBUG) || defined(_DEBUG)
  #define SMOL_DEBUG
#endif

#endif //SMOL_H
