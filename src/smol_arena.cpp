#include <smol/smol_arena.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>
#include <math.h>

namespace smol
{
  Arena::Arena(size_t initialSize):
    capacity(initialSize),
    used(0),
    data((char*) Platform::getMemory(capacity))
  {
  }

  Arena::~Arena()
  {
    Platform::freeMemory(data, capacity);
  }

  char* Arena::pushSize(size_t size)
  {
    if (used + size >= capacity)
    {
      // Grow exponentially unill we get enough space
      size_t newCapacity = (size_t) pow(2, ceil(log((double)(capacity + size)) / log(2)));

      data = (char*) Platform::resizeMemory(data, newCapacity);
      capacity = newCapacity;
    }

    char* memPtr = data + used;
    used += size;
    return memPtr;
  }

  void Arena::reset()
  {
    used = 0;
  }
}
