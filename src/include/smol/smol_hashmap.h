#ifndef SMOL_HASHMAP_H
#define SMOL_HASHMAP_H

#include <smol/smol_engine.h>
#include <smol/smol_arena.h>
#include <smol/smol_log.h>
#include <string.h>

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

  constexpr uint64 stringToHash(const char* str)
  {
    uint64 hash = 18459509;

    for(; *str; ++str)
    {
      hash += *str;
      hash += (hash << 10);
      hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
  }

  template <typename K, typename V>
  struct HashmapEntry
  {
    K* key;
    V* value;
    HashmapEntry* next;
  };

  template <typename T>
  static inline constexpr uint64 typeToHash(T value)
  {
    uint64 hash = 18459509;
    const size_t sizeOfT = sizeof(T);
    char* str = (char*) &value;

    for (int i = 0; i < sizeOfT; i++)
    {
      hash += *str;
      hash += (hash << 10);
      hash ^= (hash >> 6);
    }

    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);

    return hash;
  }

  template <typename Key, typename Value, typename Comparator = decltype(strncmp), typename Hasher = uint64 (*)(Key)>
    struct Hashmap
    {
      size_t capacity;
      int count;
      Arena keys;
      Arena values;
      Hasher hasher;

      Hashmap(size_t initialCpacity = 64, Comparator = strncmp, Hasher = smol::stringToHash);
      void add(Key key, Value value);
      Value get(Key key);
      bool hasKey(Key key);
    };

  template <typename Key, typename Value, typename Comparator, typename Hasher>
    Hashmap<Key, Value, Comparator, Hasher>::Hashmap(size_t initialCapacity, Comparator comparator, Hasher hasher):
      capacity(initialCapacity),
      count(0),
      keys(Arena(initialCapacity * sizeof(Key))),
      values(Arena(initialCapacity * sizeof(HashmapEntry<Key, Value>))),
      hasher(hasher)
  { 
    values.pushSize(initialCapacity * sizeof(Value));
  }

  template <typename Key, typename Value, typename Comparator, typename Hasher>
    void Hashmap<Key, Value, Comparator, Hasher>::add(Key key, Value value)
    {
      count++;
      uint64 hash = hasher(key);
      int index = (int) (hash % capacity);
      HashmapEntry<Key, Value>* ptrEntry = (((HashmapEntry<Key, Value>*) values.data) + index);
      char* ptr = (char*) &ptrEntry->value;
      memcpy(ptr, &value, sizeof(HashmapEntry<Key, Value>));
    }

  template <typename Key, typename Value, typename Comparator, typename Hasher>
    Value Hashmap<Key, Value, Comparator, Hasher>::get(Key key)
    {
      uint64 hash = hasher(key);
      int index = (int) (hash % capacity);
      HashmapEntry<Key, Value>* entry = (((HashmapEntry<Key, Value>*) values.data) + index);
      return *entry->value;
    }

  template <typename Key, typename Value, typename Comparator, typename Hasher>
    bool Hashmap<Key, Value, Comparator, Hasher>::hasKey(Key key)
    {
      uint64 hash = hasher(key);
      int index = (int) (hash % capacity);
      HashmapEntry<Key, Value>* entry = (((HashmapEntry<Key, Value>*) values.data) + index);
      return entry->value != nullptr;
    }

#undef SMOL_HASHMAP_VALUE_SIGNATURE
}
#endif  // SMOL_HASHMAP_H
