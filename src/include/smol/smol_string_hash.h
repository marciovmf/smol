#ifndef SMOL_STRING_HASH_H
#define SMOL_STRING_HASH_H

#include <smol.h>

namespace smol
{
  constexpr size_t strlen(const char* string)
  {
    const char* p = string;
    while(*p)
    {
      ++p;
    }

    return p - string;
  }

  constexpr size_t stringToHash(const char* str)
  {
    size_t stringLen = strlen((char*)str);
    size_t hash = 0;

    for(; *str; ++str)
    {
      hash += *str;
      hash += (hash << 10);
      hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    hash ^= stringLen;

    return hash;
  }
}
#endif //SMOL_STRING_HASH_H

