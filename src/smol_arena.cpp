#include <smol/smol_arena.h>
#include <smol/smol_platform.h>

namespace smol
{
  Arena::Arena(size_t initialSize)
  {
    //TODO(marcio): Make sure to get aligned memory here.
    size_t minimumSize = MEGABYTE(5);
    capacity = initialSize < minimumSize ? minimumSize : initialSize;
    used = 0;
    data = (char*) Platform::getMemory(capacity);
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
      size_t newCapacity = capacity;
      size_t extraSpace = 0;
      do
      {
        newCapacity = newCapacity << 1;
        extraSpace = newCapacity - used;
      }
      while (size > extraSpace);

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
