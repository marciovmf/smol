#ifndef SMOL_MATERIAL_H
#define SMOL_MATERIAL_H

#include <smol/smol_engine.h>
#include <smol/smol_handle_list.h>
#include <smol/smol_shader.h>
#include <smol/smol_vector2.h>
#include <smol/smol_vector3.h>
#include <smol/smol_vector4.h>
#include <smol/smol_texture.h>

namespace smol
{
  struct SMOL_ENGINE_API MaterialParameter : public ShaderParameter
  {
    union
    {
      float floatValue;
      Vector2 vec2Value;
      Vector3 vec3Value;
      Vector4 vec4Value;
      int32 intValue;
      uint32 uintValue;
    };
  };

  struct SMOL_ENGINE_API Material
  {
    enum
    {
      MAX_TEXTURES = 6,
      MAX_NAME_LEN = 255
    };

    enum DepthTest
    {
      DISABLE         = 0,
      LESS            = 1,
      LESS_EQUAL      = 2,
      EQUAL           = 3,
      GREATER         = 4,
      GREATER_EQUAL   = 5,
      DIFFERENT       = 6,
      NEVER           = 7,
      ALWAYS          = 8
    };

    enum CullFace
    {
      NONE            = 0,
      BACK            = 1,
      FRONT           = 2,
      FRONT_AND_BACK  = 3
    };

    char name[MAX_NAME_LEN];
    Handle<ShaderProgram> shader;
    Handle<Texture> textureDiffuse[MAX_TEXTURES];
    int diffuseTextureCount;
    int renderQueue;
    MaterialParameter parameter[SMOL_MAX_SHADER_PARAMETERS];
    int parameterCount;
    DepthTest depthTest;
    CullFace cullFace;

    Material& setSampler2D(const char* name, Handle<Texture> handle);
    Material& setUint(const char* name, unsigned int value);
    Material& setInt(const char* name, int value);
    Material& setFloat(const char* name, float value);
    Material& setVec2(const char* name, const Vector2& value);
    Material& setVec3(const char* name, const Vector3& value);
    Material& setVec4(const char* name, const Vector4& value);

    private:
    MaterialParameter* getParameter(const char* name, ShaderParameter::Type type);
  };

  template class SMOL_ENGINE_API smol::HandleList<smol::Material>;
  template class SMOL_ENGINE_API smol::Handle<smol::Material>;
}
#endif  // SMOL_MATERIAL_H
