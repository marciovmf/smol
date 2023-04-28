#ifndef SMOL_RANDOM_H
#define SMOL_RANDOM_H
#include <smol/smol.h>

namespace smol
{
  void seed(int32);
  double random01();
  int32 randomRange(int32 min, int32 max);
}

#endif  // SMOL_RANDOM_H
