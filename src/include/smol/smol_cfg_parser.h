#ifndef SMOL_CFG_PARSER
#define SMOL_CFG_PARSER

#include <smol/smol_engine.h>
#include <smol/smol_arena.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector4.h>

#define SMOL_CONFIG_VAR_MAX_NAME_LEN 64

namespace smol
{
  struct SMOL_ENGINE_API ConfigVariable
  {
    enum Type { STRING, NUMBER, VECTOR2, VECTOR3, VECTOR4 };

    const char* name;
    int64 hash;
    Type type;
    union
    {
      double numberValue;
      float vec2Value[2];
      float vec3Value[3];
      float vec4Value[4];
      const char* stringValue;
    };
  };

  struct SMOL_ENGINE_API ConfigEntry
  {
    uint32 variableCount;
    ConfigVariable* variables;
    ConfigEntry* next;
    const char* name;
    int64 hash;

    double getVariableNumber(const char* name, double defaultValue = 1.0f, bool warn = false) const;
    Vector4 getVariableVec4(const char* name, Vector4 defaultValue = {0.0f, 0.0f, 0.0f, 0.0f}, bool warn = false) const;
    Vector3 getVariableVec3(const char* name, Vector3 defaultValue = {0.0f, 0.0f, 0.0f}, bool warn = false) const;
    Vector2 getVariableVec2(const char* name, Vector2 defaultValue = {0.0f, 0.0f}, bool warn = false) const;
    const char* getVariableString(const char* name, const char* defaultValue = nullptr, bool warn = false) const;

  };

  struct SMOL_ENGINE_API Config
  {
    smol::Arena arena;
    char* buffer;
    ConfigEntry* entries;
    uint32 entryCount;

    Config(size_t initialArenaSize);
    Config(const char* path, size_t initialArenaSize = MEGABYTE(1));
    const ConfigEntry* findEntry(const char *name, const ConfigEntry* start = nullptr) const;
    ~Config();
    bool load(const char* path);
  };
}

#endif  // SMOL_CFG_PARSER
