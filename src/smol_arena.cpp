#include <smol/smol_arena.h>
#include <smol/smol_platform.h>
#include <smol/smol_engine.h>
#include <smol/smol_log.h>
#include <math.h>

// Note the Arena does not care about constructors or destructors of the objects
// it stores. For the arena anything it stores is plain data.
// Keep that in mind when using it.

namespace smol
{

  Arena::Arena():
    used(0), capacity(0), data(nullptr) { }

  Arena::Arena(size_t initialSize) 
  {
    initialize(initialSize); 
  }

  Arena::Arena(Arena&& other)
  { 
    this->~Arena();

    capacity = other.capacity;
    used = other.used;
    data = other.data;

    other.data = nullptr;
    other.used = 0;
    other.capacity = 0;
  }

  Arena::~Arena() 
  {
    Platform::freeMemory(data, capacity); 
  }

  void Arena::initialize(size_t initialSize)
  {
    capacity = initialSize;
    used = 0;
    if (initialSize > 0)
      data = (char*) Platform::getMemory(capacity);
    else
      data = nullptr;
  }

  char* Arena::pushSize(size_t size)
  {
    if (used + size >= capacity)
    {
      size_t newCapacity = 2 * (capacity + size);
      // get next pow2 larger than current capacity
      newCapacity = (newCapacity >> 1) | newCapacity;
      newCapacity = (newCapacity >> 2) | newCapacity;
      newCapacity = (newCapacity >> 4) | newCapacity;
      newCapacity = (newCapacity >> 8) | newCapacity;
      newCapacity = (newCapacity >> 16) | newCapacity;
      newCapacity = (newCapacity >> 32) | newCapacity;
      newCapacity++;
      data = (char*) Platform::resizeMemory(data, newCapacity);
      capacity = newCapacity;
    }

    char* memPtr = data + used;
    used += size;
    return memPtr;
  }

  inline void Arena::reset() { used = 0; }

  inline size_t Arena::getCapacity() const { return capacity; }

  inline size_t Arena::getUsed() const { return used; }

  const char* Arena::getData() const { return data; }
}
