#ifndef SMOL_CFG_PARSER
#define SMOL_CFG_PARSER

#include <smol/smol_engine.h>
#include <smol/smol_arena.h>

#define SMOL_CONFIG_VAR_MAX_NAME_LEN 64

namespace smol
{
  struct SMOL_ENGINE_API ConfigVariable
  {
    enum Type { STRING, NUMBER, VECTOR2, VECTOR3, VECTOR4 };

    const char* name;
    Type type;
    union
    {
      float numberValue;
      float vec2Value[2];
      float vec3Value[3];
      float vec4Value[4];
      const char* stringValue;
    };
  };

  struct SMOL_ENGINE_API ConfigEntry
  {
    int variableCount;
    ConfigVariable* variables;
    ConfigEntry* next;
  };

  struct SMOL_ENGINE_API Config
  {
    smol::Arena arena;
    char* buffer;
    ConfigEntry* entries;
    int entryCount;

    Config(size_t initialArenaSize);
    Config(const char* path, size_t initialArenaSize = MEGABYTE(1));
    bool load(const char* path);
  };
}

#endif  // SMOL_CFG_PARSER
