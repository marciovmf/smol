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

    float getVariableNumber(const char* name, float defaultValue = 1.0f);
    Vector4 getVariableVec4(const char* name, Vector4 defaultValue = {0.0f, 0.0f, 0.0f, 0.0f});
    Vector3 getVariableVec3(const char* name, Vector3 defaultValue = {0.0f, 0.0f, 0.0f});
    Vector2 getVariableVec2(const char* name, Vector2 defaultValue = {0.0f, 0.0f});
    const char* getVariableString(const char* name, const char* defaultValue = nullptr);

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
