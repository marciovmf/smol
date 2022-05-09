#ifndef SMOL_ARENA
#define SMOL_ARENA
#include <smol/smol_engine.h>

#define KILOBYTE(value) (size_t) ((value) * 1024LL)
#define MEGABYTE(value) (size_t) (KILOBYTE(value) * 1024LL)
#define GIGABYTE(value) (size_t) (MEGABYTE(value) * 1024LL)

namespace smol
{
  struct SMOL_ENGINE_API Arena
  {
    size_t capacity;
    size_t used;
    char* data;

    Arena();
    Arena(size_t initialSize);
    void initialize(size_t initialSize);
    ~Arena();
    char* pushSize(size_t size);
    void reset();

  };
}

#endif  // SMOL_ARENA
