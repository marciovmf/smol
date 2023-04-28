
#include <smol/smol_random.h>
#include <stdlib.h>

namespace smol
{
  void seed(int32 seed)
  {
    srand(seed);
  }

  double random01()
  {
    return ((double) rand() / (RAND_MAX)) + 1;
  }

  int32 randomRange(int32 min, int32 max)
  {
    return (rand() % (max - min + 1)) + min;
  }
}
