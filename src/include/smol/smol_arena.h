#ifndef SMOL_ARENA
#define SMOL_ARENA
#include <smol/smol_engine.h>

#define KILOBYTE(value) (size_t) ((value) * 1024LL)
#define MEGABYTE(value) (size_t) (KILOBYTE(value) * 1024LL)
#define GIGABYTE(value) (size_t) (MEGABYTE(value) * 1024LL)

#define SMOL_ARENA_DEFAULT_SIZE (MEGABYTE(64))

namespace smol
{
  struct SMOL_ENGINE_API Arena
  {
    size_t capacity;
    size_t used;
    char* data;

    Arena(size_t initialSize = SMOL_ARENA_DEFAULT_SIZE);
    ~Arena();
    char* pushSize(size_t size);
    void reset();
  };
}

#endif  // SMOL_ARENA
